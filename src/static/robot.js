var lastPacket = new Date().getTime()
var lastPacketDelta=0;
var manager = nipplejs.create({
  zone: document.getElementById('joypad'),
  mode: 'static',
  position: {
    size: 200,
    left: '50%',
    bottom: '22%'
  },
  color: 'blue'
});
let socket=null;
console.log(location.host);
if (location.host=="127.0.0.1:3000" || location.host=="localhost:3000" ){
  console.log("Connecting to DEBUG ESP")
  socket = new WebSocket("ws://192.168.86.45/test");
} else {
  socket = new WebSocket("ws://" + location.host + "/test");
}

socket.binaryType = 'arraybuffer';
socket.onopen = function(e) {
  console.log("[open] Connection established");
  console.log("Sending to server");
  setInterval(updateJoystick, 60);
  setInterval(updateLastPacket, 500);
  setInterval(updateValues, 60);
  setInterval(sendHeartbeat, 1000);
};

var telMap = new Map();
socket.onmessage = function(event) {
  //console.log(`[message] Data received from server: ${event.data}`);
  //console.log(event.data);
  let bufferInt32 = new Int32Array(event.data);
  let bufferFloat = new Float32Array(event.data);
  let bufferInt8 = new Int8Array(event.data);
  let command = bufferInt32[0];
  //console.log("<"+String(command));
  switch (command) {
    case 96:
      let motor = bufferInt32[1];
      let p     = bufferFloat[2];
      let i     = bufferFloat[3];
      let d     = bufferFloat[4];
      console.log("Got PID for motor" +String(motor) + "\t p:" + String(p) +"\t"+ ": i:" + String(i) +"\t"+ ": d:" + String(d) +"\t");
      break;
    case 80: // Heartbeat Ping
      resetHeartbeat(bufferInt32[1]);
      break;
    case 17: // Console Data
      let blen = bufferInt32[1];
      let webcon="";
      for (i=8; i<blen+8; i++){
        let ch = bufferInt8[i];
        webcon = webcon + String.fromCharCode(ch);
      }
      webConsolePrint(webcon);
      break;
    case 30: //Bulk Value Update
      let numValues = bufferInt32[1];
      for (let i = 0; i < numValues; i++) {
        let index = bufferInt32[((i + 1) * 2)];
        let value = bufferFloat[((i + 1) * 2) + 1];
        //console.log("Value\ti: " + String(index) +"\tv:"  + String(value));
        // Lookup slot by index
        // If it's un inited, init it.
        if (telMap.get(index) == undefined) {
          telMap.set(index, {
            'name': '',
            'data': 0.0
          });
        }
        // Update Value
        telMap.get(index).data = value;
      }
      break;
    case 29: //Bulk Label Update
      //console.log("New Labels Update!")
      let labelCount = bufferInt32[1];
      let stringDataStart = bufferInt32[2];

      // walk manifest
      for (let i = 0; i < labelCount; i++) {
        let labeltext = "";
        let index = bufferInt32[((i + 1) * 3) + 0];
        let offset = bufferInt32[((i + 1) * 3) + 1];
        let length = bufferInt32[((i + 1) * 3) + 2];
        for (j = stringDataStart + offset; j < stringDataStart + offset + length; j++) {
          let ch = bufferInt8[j];
          if (ch != 0) labeltext = labeltext + String.fromCharCode(ch);
        }
        //console.log("Label\ti: " + String(index) + "\to: " + String(offset) + "\tl: " + String(length) + "\t '" + labeltext + "'");
        if (telMap.get(index) == undefined) {
          telMap.set(index, {
            'name': '',
            'data': 0.0
          });
        }
        // Update Value
        telMap.get(index).name = labeltext;
      }
      break;
    default:
      console.log("Unknown Packet");
      console.log(event.data);
      break;


  }

  //console.log( new Date().getTime() - lastPacket);
  lastPacketDelta=new Date().getTime()-lastPacket;
  lastPacket = new Date().getTime();

};

socket.onclose = function(event) {
  if (event.wasClean) {
    console.log(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
    setTimeout(location.reload(), 15000);
  } else {
    // e.g. server process killed or network down
    // event.code is usually 1006 in this case
    console.log('[close] Connection died');
    setTimeout(location.reload(), 15000);
  }
};

socket.onerror = function(error) {
  console.log(`[error] ${error.message}`);
  setTimeout(location.reload(), 15000);
};
var joydata = 0;
var newData = false;
manager.get(0).on('move start end', function(evt, data) {
  joydata = data;
  newData = true;

});

function sendSlider(num, value) {
  // build the packet
  var ab = new ArrayBuffer(12);
  var abFloat = new Float32Array(ab);
  var abInt = new Uint32Array(ab);
  abInt[0] = 48; // js packet
  abFloat[1] = num;
  abFloat[2] = value / 100.0;
  socket.send(ab);
//console.log(">48");
}

function updateValues() {
  var contents = "";
  for (let [k, v] of telMap.entries()) {
    contents += "<tr><td>" + v.name + "</td><td>" + v.data + "</td></tr>";
  }
  contents += "</table>"
  document.getElementById("telemetry").innerHTML = contents;
}

function updateLastPacket(){
  lastPacketDelta=new Date().getTime()-lastPacket;
  let elem = document.getElementById("lastPacket")
  elem.innerHTML = String(lastPacketDelta);
  if (lastPacketDelta>500){
    elem.classList.add("warning-text");
  } else {
    elem.classList.remove("warning-text");
  }
}

var pingUUID=0; // UUID of last sent ping
var droppedPings=0; // number of unanswered pings
var packetTxTime=0; // unix time packet was sent
var pingTime=0;  // round trip time in ms
function sendHeartbeat(){

  if (pingUUID!=0){
    // we've come around again and UUID has not been reset.
    // Ping never came back.
    droppedPings++;
  }
  pingUUID=parseInt((Math.random()*(2147483648))+1); // random int. never zero.
  var ab = new ArrayBuffer(8);
  var abInt = new Uint32Array(ab);
  abInt[0] = 80; // HB packet
  abInt[1] = pingUUID; // HB packet
  packetTxTime=new Date().getTime();
  socket.send(ab);
//console.log(">80");
  //console.log("Heartbeat Sent "+String(pingUUID));
}

function resetHeartbeat(packet_uuid){
  //console.log("Heartbeat RX'd "+String(packet_uuid));
  if (packet_uuid==pingUUID){

        pingTime=new Date().getTime()-packetTxTime;
        let elem= document.getElementById("hb_ping_ms");
        elem.innerHTML = pingTime;
        if (pingTime>100){
          elem.classList.add("warning-text");
        } else {
          elem.classList.remove("warning-text");
        }
        pingUUID=0;
  }
}

function updateJoystick() {
  if (newData == true) {
    newData = false;
    //console.log("update");
    joydata_old = joydata;
    var nDist = 0.0;
    if (joydata.distance) {
      nDist = joydata.distance / 50.0
    }
    var Angle = 0.0;
    if (joydata.angle) {
      Angle = joydata.angle.radian;
    }
    // build the packet
    var ab = new ArrayBuffer(20);
    var abFloat = new Float32Array(ab);
    var abInt = new Uint32Array(ab);
    abInt[0] = 32; // js packet
    abFloat[1] = 1
    abFloat[2] = 1
    abFloat[3] = Angle;
    abFloat[4] = nDist;
    socket.send(ab);
  //  console.log(">32");
  }
}

function webConsolePrint(data){
  contents = document.getElementById("consoletext").innerHTML
  document.getElementById("consoletext").innerHTML = contents + data;
}

function updatePIDValuesUI(){
  m1s_elem  = document.getElementById("m1s");
  m1p_elem  = document.getElementById("m1p");
  m1i_elem  = document.getElementById("m1i");
  m1d_elem  = document.getElementById("m1d");
  m2s_elem  = document.getElementById("m2s");
  m2p_elem  = document.getElementById("m2p");
  m2i_elem  = document.getElementById("m2i");
  m2d_elem  = document.getElementById("m2d");
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'myservice/username?id=some-unique-id');
  xhr.onload = function() {
    if (xhr.status === 200) {

    }
  };
  xhr.send();
  // Get current values.
}

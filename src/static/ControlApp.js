// Setup Packet Types
const packet_type = {
  single_telemetry_value_update: 16,
  console_update: 17,
  bulk_telemetry_label_update: 29,
  bulk_telemetry_value_update: 30,
  single_telemetry_value_update: 31,
  joystick_update: 32,
  slider_update: 48,
  button_update: 64,
  heartbeat: 80,
  motor_pid_values: 96,
  motor_setpoint: 97
}

const application_states = {
  connecting:       0,
  connected:        1,
  disconnected:     2,
  invalid:          3
}




function socket_message(event) {
  decode_packet(event.data);
}

function socket_disconnected(event) {
  clearInterval(interval_update_joystick);
  clearInterval(interval_update_last_packet_counter);
  clearInterval(interval_update_ui_values);
  clearInterval(interval_send_heartbeat);
  application_state=application_states.disconnected;
  if (event.wasClean) {
    console.log(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
  } else {
    console.log('[close] Connection died');
  }
}

function socket_connected(event) {
  console.log("Websocket Connection Established");
  send_heartbeat(); // send out one HB so ui doesn't display blanks untill the timer triggers.
  interval_update_joystick = setInterval(update_joystick, 60);
  interval_update_last_packet_counter = setInterval(update_last_packet_counter, 500);
  interval_update_ui_values = setInterval(update_ui_values, 60);
  interval_send_heartbeat = setInterval(send_heartbeat, 5000);
  application_state=application_states.connected;
}

function socket_error(event) {
  console.log(`[error] ${event.message}`);
}

function update_joystick() {
  if (joystick_has_data == true) {

    joystick_has_data = false;
    //console.log("update");
    let nDist = 0.0;
    if (joystick_data.distance) {
      nDist = joystick_data.distance / 50.0
    }
    let Angle = 0.0;
    if (joystick_data.angle) {
      Angle = joystick_data.angle.radian;
    }
    // build the packet
    let ab = new ArrayBuffer(20);
    let abFloat = new Float32Array(ab);
    let abInt = new Uint32Array(ab);
    abInt[0] = 32; // js packet
    abFloat[1] = 1
    abFloat[2] = 1
    abFloat[3] = Angle;
    abFloat[4] = nDist;
    robot_socket.send(ab);
    //  console.log(">32");
  }
}

function update_last_packet_counter() {
  last_packet_delta = new Date().getTime() - last_packet_datetime;
  dom_last_packet_display.innerHTML = String(last_packet_delta);
  if (last_packet_delta > 500) {
    dom_last_packet_display.classList.add("warning-text");
  } else {
    dom_last_packet_display.classList.remove("warning-text");
  }
}

function update_ui_values() {
  let contents = "";
  for (let [k, v] of telemetry_values.entries()) {
    contents += "<tr><td>" + v.name + "</td><td>" + v.data + "</td></tr>";
  }
  contents += "</table>"
  dom_telemetry.innerHTML = contents;
}

function send_heartbeat() {
  if (heartbeat_pingUUID != 0) {
    // we've come around again and UUID has not been reset.
    // Ping never came back.
    heartbeat_droppedPings++;
  }
  heartbeat_pingUUID = parseInt((Math.random() * (2147483648)) + 1); // random int. never zero.
  var ab = new ArrayBuffer(8);
  var abInt = new Uint32Array(ab);
  abInt[0] = 80; // HB packet
  abInt[1] = heartbeat_pingUUID; // HB packet
  heartbeat_packetTxTime = new Date().getTime();
  console.log("Heartbeat TX'd "+String(heartbeat_pingUUID));
  robot_socket.send(ab);
}

function decode_heartbeat(data){
  let buffer_int = new Int32Array(data);
  let uuid=buffer_int[1];
  console.log("Heartbeat RX'd "+String(heartbeat_pingUUID));
  if (heartbeat_pingUUID==uuid){
        console.log("match");
        heartbeat_pingTime=new Date().getTime()-heartbeat_packetTxTime;
        dom_pingtime_display.innerHTML = heartbeat_pingTime;
        if (heartbeat_pingTime>100){
          dom_pingtime_display.classList.add("warning-text");
        } else {
          dom_pingtime_display.classList.remove("warning-text");
        }
        heartbeat_pingUUID=0;
  }
}

function decode_motor_setpoint(data){
  // Create 3 mirror arrays.
  let buffer_int = new Int32Array(data);
  let buffer_float = new Float32Array(data);
  let buffer_byte = new Int8Array(data);

  let motor = buffer_int[1];
  let set   = buffer_float[2];

  switch(motor){
    case 0:
      dom_motor1_setpoint.value=String(set.toFixed(5));
      break;
    case 1:
      dom_motor2_setpoint.value=String(set.toFixed(5));
  }
}

function decode_motor_pid(data){
  // Create 3 mirror arrays.
  let buffer_int = new Int32Array(data);
  let buffer_float = new Float32Array(data);
  let buffer_byte = new Int8Array(data);


  let motor = buffer_int[1];
  let p     = buffer_float[2];
  let i     = buffer_float[3];
  let d     = buffer_float[4];

  //console.log("Got PID for motor" +String(motor) + "\t p:" + String(p) +"\t"+ ": i:" + String(i) +"\t"+ ": d:" + String(d) +"\t");

  switch(motor){
    case 0:
      dom_motor1_p.value=String(p.toFixed(5));
      dom_motor1_i.value=String(i.toFixed(5));
      dom_motor1_d.value=String(d.toFixed(5));
      break;
    case 1:
      dom_motor2_p.value=String(p.toFixed(5));
      dom_motor2_i.value=String(i.toFixed(5));
      dom_motor2_d.value=String(d.toFixed(5));
  }

  //

}

function decode_console_update(data){
  let buffer_int = new Int32Array(data);
  let buffer_byte = new Int8Array(data);
  let length=buffer_int[1];

  console_contents;
  for(let i=0; i<length; i++){
    let char = buffer_byte[i+8];
    let strc = String.fromCharCode(buffer_byte[i+8]);
    //console.log(buffer_byte);
    if (char==10) strc="<br />";
    console_contents += strc;
  }
  dom_console_window.innerHTML = console_contents + "<span id=\"anchor\"> </span>";;

}

function decode_packet(data) {
  // Reset Packet Timers
  last_packet_delta=new Date().getTime()-last_packet_datetime;
  last_packet_datetime = new Date().getTime();

  // Exctact Command Code
  let buffer_int = new Int32Array(data);
  let command = buffer_int[0];

  // Decode packet based on command code
  switch (command) {
    case packet_type.motor_pid_values:
      decode_motor_pid(data);
      break;

    case packet_type.motor_setpoint:
      decode_motor_setpoint(data);
      break;

    case packet_type.heartbeat:
      decode_heartbeat(data);
      break;

    case packet_type.console_update:
      decode_console_update(data);
      break;

    case packet_type.bulk_telemetry_value_update:
      decode_bulk_telemetry_value_update(data);
      break;

    case packet_type.bulk_telemetry_label_update:
      decode_bulk_telemetry_label_update(data);
      break;


    default:
      console.log("Inknown command byte '${command}'");
      console.log(data);
      break;
  }
}


function decode_bulk_telemetry_label_update(data) {
  // Create 3 mirror arrays.
  let buffer_int = new Int32Array(data);
  let buffer_float = new Float32Array(data);
  let buffer_byte = new Int8Array(data);

  // Extract number of labels and start of string data from manifest.
  let labelCount = buffer_int[1];
  let stringDataStart = buffer_int[2];

  // walk manifest extracting strings.
  for (let i = 0; i < labelCount; i++) {
    let labeltext = "";
    let index = buffer_int[((i + 1) * 3) + 0];
    let offset = buffer_int[((i + 1) * 3) + 1];
    let length = buffer_int[((i + 1) * 3) + 2];
    for (j = stringDataStart + offset; j < stringDataStart + offset + length; j++) {
      let ch = buffer_byte[j];
      if (ch != 0) labeltext = labeltext + String.fromCharCode(ch);
    }
    //console.log("Label\ti: " + String(index) + "\to: " + String(offset) + "\tl: " + String(length) + "\t '" + labeltext + "'");
    // If there is no label at this index, create a telemetry object.
    if (telemetry_values.get(index) == undefined) {
      telemetry_values.set(index, {
        'name': '',
        'data': 0.0
      });
    }
    // Update existing Telemetry Object
    telemetry_values.get(index).name = labeltext;
  }
}

function decode_bulk_telemetry_value_update(data) {
  // Create 3 mirror arrays.
  let buffer_int = new Int32Array(data);
  let buffer_float = new Float32Array(data);
  let buffer_byte = new Int8Array(data);

  // Get number of bundled telemetry values
  let numValues = buffer_int[1];

  // Extract values from list
  for (let i = 0; i < numValues; i++) {
    let index = buffer_int[((i + 1) * 2)];
    let value = buffer_float[((i + 1) * 2) + 1];
    //console.log("Value\ti: " + String(index) +"\tv:"  + String(value));

    // If no telemetry ovject exists at a value's index, create one.
    if (telemetry_values.get(index) == undefined) {
      telemetry_values.set(index, {
        'name': '',
        'data': 0.0
      });
    }

    // Update Existing object with updated telemetry value
    telemetry_values.get(index).data = value;
  }
}


function send_slider(num,value){
  // build the packet
  var ab = new ArrayBuffer(12);
  var abFloat = new Float32Array(ab);
  var abInt = new Uint32Array(ab);
  abInt[0] = 48; // js packet
  abFloat[1] = num;
  abFloat[2] = value / 100.0;
  robot_socket.send(ab);
}

function initialize_app() {
  application_state = application_states.connecting;
  if (robot_socket != null) robot_socket.close();
  if (location.host == "127.0.0.1:3000" || location.host == "localhost:3000") {
    console.log("App running on localhost.. Connecting to debug ESP")
    robot_socket = new WebSocket("ws://192.168.86.45/test");
  } else {
    robot_socket = new WebSocket("ws://" + location.host + "/test");
  }



  robot_socket.binaryType = 'arraybuffer';
  robot_socket.onopen = socket_connected;
  robot_socket.onclose = socket_disconnected;
  robot_socket.onerror = socket_error;
  robot_socket.onmessage = socket_message;


}

function handle_application_state(){
  if (last_packet_delta>10000){
    robot_socket.close()
    clearInterval(interval_update_last_packet_counter);
    last_packet_delta=0;
  }
  if (application_state==application_state_last) return;
  application_state_last=application_state;
  console.log("App state changed");
  switch(application_state){
    case application_states.connecting:
      dom_overlay_disconnected.classList.add("hide");
      dom_overlay_connecting.classList.remove("hide");
      break;

    case application_states.connected:
      dom_overlay_disconnected.classList.add("hide");
      dom_overlay_connecting.classList.add("hide");
      break;

    case application_states.disconnected:
      dom_overlay_disconnected.classList.remove("hide");
      dom_overlay_connecting.classList.add("hide");
      console.log("Attempting to reconnect with device..");
      setTimeout(initialize_app, 5000);
      break;
  }
}

// Data Structure to hold telemetry numValues
telemetry_values = new Map();

// our itnerval Timers
var interval_update_joystick = 0;
var interval_update_last_packet_counter = 0;
var interval_update_ui_values = 0;
var interval_send_heartbeat = 0;



// Colect all our DOM Elements
var dom_joypad = document.getElementById('joypad');
var dom_telemetry = document.getElementById("telemetry");
var dom_last_packet_display = document.getElementById("lastPacket");
var dom_pingtime_display = document.getElementById("hb_ping_ms");
var dom_console_window = document.getElementById("consoletext");
var dom_overlay_disconnected        = document.getElementById("overlay_disconnected");
var dom_overlay_connecting        = document.getElementById("overlay_connecting");

var dom_motor1_setpoint = document.getElementById("m1s");
var dom_motor1_p        = document.getElementById("m1p");
var dom_motor1_i        = document.getElementById("m1i");
var dom_motor1_d        = document.getElementById("m1d");

var dom_motor2_setpoint = document.getElementById("m2s");
var dom_motor2_p        = document.getElementById("m2p");
var dom_motor2_i        = document.getElementById("m2i");
var dom_motor2_d        = document.getElementById("m2d");

// Application State
var application_state_last = application_states.invalid;
var application_state = application_states.connecting;

// Set up Heartbeat variables.
var heartbeat_pingUUID = 0; // UUID of last sent ping
var heartbeat_droppedPings = 0; // number of unanswered pings
var heartbeat_packetTxTime = 0; // unix time packet was sent
var heartbeat_pingTime = 0; // round trip time in ms

// Set up vars for last Packet Counter
var last_packet_datetime = new Date().getTime()
var last_packet_delta=0;

//set up Joystick variables
var joystick_data = 0;
var joystick_has_data = false;

// contents of serial windoe
var console_contents = dom_console_window.innerHTML;

// Setup our connection.
var robot_socket = null;

// Setup joystick
var joypad_manager = nipplejs.create({
  zone: dom_joypad,
  mode: 'static',
  position: {
    size: 200,
    left: '50%',
    bottom: '22%'
  },
  color: 'blue'
});
joypad_manager.get(0).on('move start end', function(evt, data) {
  joystick_data = data;
  joystick_has_data = true;

});

setInterval(handle_application_state,500);

initialize_app();

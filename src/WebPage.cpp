#include <Arduino.h>
#include <WebServer.h>
#include <HTTP_Method.h>
#include "WebPage.h"
#include "static/indexhtml.h"
#include "static/nipplejsminjs.h"
#include <ESPAsyncWebServer/ESPAsyncWebServer.h>
#include "Motor.h"
#include "RBE1001Lib.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/test");
static WebPage *thisPage;
static char stringBuffer[200];
//static uint8_t buffer[labelbuflen];
const String updtime="Uptime";
const String js(nipplejs_min_js);
const String myHTML(index_html);

static bool lockOutSending = false;

long timeSinceLastSend =0;

const char *strings[12] = { "Left Encoder Degrees","Left Encoder Effort","Left Encoder Degrees-sec",
		"Right Encoder Degrees","Right Encoder Effort","Right Encoder Degrees-sec" ,
				"2 Encoder Degrees","2 Encoder Effort","2 Encoder Degrees-sec" ,
				"3 Encoder Degrees","3 Encoder Effort","3 Encoder Degrees-sec"
};

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  uint32_t *asInt = (uint32_t *)data;
  thisPage->rxPacketCount++;
  float    *asFloat = (float *)data;
  if(type == WS_EVT_CONNECT){
    //Serial.println("Websocket client connection received");
	  thisPage->markAllDirty();
	  thisPage->SendAllLabels();
	  delay(20);
	  thisPage->SendAllValues();
  } else if(type == WS_EVT_DISCONNECT){
    //Serial.println("Client disconnected");

  } else if(type == WS_EVT_DATA){
	  if (len<3) {
		  //Serial.println("Packet too short!");
		  return;
	  }
	  uint32_t command = asInt[0];
	  /*
    // Data Format
    // 4B: Message Type
	//		0x10 (16)	Value Update
	//  	  	4B: value index
	//			4B: value data
	 * 		0x1d (29)	Bulk Label Update
	 * 			4B:	Number of Labels in this update
	 * 			4B: xx
	 * 			[repeated next 12B block for each label]
	 * 			4B: Index to update
	 * 			4B: String Offset in packet
	 * 			4B: String Length
	 *
	 * 			nB: [all label strings concatenated ]
	 *
	//  	0x1e (30)	Bulk Value Update
	//	  	  	4B: Number of Values
	 * 			[repeat for all values]
	//  	    4B:	value index n
	//			4B:	value data n
	//
	//		0x1f (31)	New Value
	//			4B: value index
	//			*B: value name
    // 		0x20 (32)	Joystick Update (size 5*4 20 bytes)
    //			4B: Position X  (float, 0.0-1.0)
	//			4B: Position Y  (float, 0.0-1.0)
	//			4B: Angle       (float, radians)
	//			4B: Magnitude   (float, 0.0-1.0)
    // 		0x30 (48)	Slider Update
    //			4B: Slider Number Uint32
	//			4B: Slider Value (float, 0.0-1.0)
    //      0x40 (64)	Button Update
    //			4B: Button Number
    //			4B: Button State (0.0 or 1.0)
     *
     */

    //Serial.println("Command is: "+String(command)+"\t["+String(packetCount++)+"]");
    switch(command){
    	case 0x20:

    		thisPage->setJoystickValue(asFloat[1], asFloat[2], asFloat[3], asFloat[4]);
    		break;
    	case 0x30:{
    		//Serial.println("Slider Update");
    		//Serial.println("slider:\t"+String(asFloat[1]));
    		//Serial.println("pos:\t"+String(asFloat[2]));
    		thisPage->setSliderValue(asInt[1],asFloat[2]);
    	}
    		break;
    	case 0x40:
    		//Serial.println("Button Update");
    		break;

    }


  }
}



char* String2Chars(String str){
	str.toCharArray(stringBuffer, 200, 0);
	return stringBuffer;
}


WebPage::WebPage() {
  thisPage = this;
  joystick.xpos  = 0;
  joystick.ypos  = 0;
  joystick.angle = 0;
  joystick.mag   = 0;
  for(int i=0; i<numValues; i++) {
	  values[i].used=false;
	  values[i].value=0;
	  values[i].name=String("");
	  values[i].valueDirty=false;
	  values[i].labelDirty=false;
  }
  for(int i=0; i<numSliders; i++) sliders[i]=0;
}

bool isSafeToSend(){
	if(millis()-timeSinceLastSend>10){
		return !lockOutSending;
	}
	return false;
}
void lock(){
	while(!isSafeToSend()){
			//Serial.println("W daTA ");
			delay(1);
		}
		lockOutSending=true;
}
void unlock(){
	timeSinceLastSend=millis();
	//Serial.println("R data UnLock");
	lockOutSending=false;

}

void updateTask(void *param){
	//delay(1200);
	while(1){

		lock();
		//Serial.println("L data Lock");
		if (thisPage->SendAllLabels()) delay(20);
		thisPage->SendAllValues();
		unlock();


		for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++) {
			if (Motor::list[i] != NULL) {
				thisPage->valueChanged(strings[i*3],Motor::list[i]->getCurrentDegrees());
				thisPage->valueChanged(strings[i*3+1],Motor::list[i]->GetEffort());
				thisPage->valueChanged(strings[i*3+2],Motor::list[i]->getDegreesPerSecond());
			}
		}
		thisPage->valueChanged(updtime,((float)millis())/1000.0);
		delay(1);
	}
}

void WebPage::initalize(){
	//ESP_LOGI("WebPage::WebPage","WebPage Init..");
	server.begin();
	//Serial.println("HTTP server started");
//
    server.on("/", 0b00000001, [](AsyncWebServerRequest *request){

    	lock();
		//Serial.println("L text/html Lock");
		request->send(200, "text/html",myHTML );
		unlock();

    });
    server.on("/nipplejs.min.js", 0b00000001, [](AsyncWebServerRequest *request){

    	lock();
		//Serial.println("L text/javascript Lock");
		request->send(200, "text/javascript", js);
		unlock();

    });


    xTaskCreatePinnedToCore(
    		updateTask, /* Function to implement the task */
          "updateTask", /* Name of the task */
		  8192 * 8,  /* Stack size in words */
          NULL,  /* Task input parameter */
          4,  /* Priority of the task */
          &thisPage->updateTaskHandle,  /* Task handle. */
          1); /* Core where the task should run */

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
}

float WebPage::getSliderValue(uint32_t number){
	//Serial.println("Index:\t"+String(number));
	if (number>3) return 0.0;
	return sliders[number];
}
float WebPage::getJoystickAngle(){
	return joystick.angle;
}
float WebPage::getJoystickMagnitude(){
	return joystick.mag;
}
float WebPage::getJoystickX(){
	return joystick.xpos;
}
float WebPage::getJoystickY(){
	return joystick.ypos;
}
JoyData * WebPage::getJoystickData(){

//	Serial.print("\nJoystick Update");
//	Serial.print(" X pos:\t"+String(joystick.xpos));
//	Serial.print(" Y pos:\t"+String(joystick.ypos));
//	Serial.print(" Angle:\t"+String(joystick.angle));
//	Serial.print(" Dist:\t"+String(joystick.mag));

	return &joystick;
}

void WebPage::setSliderValue(uint32_t number, float value){
	if (number<4) thisPage->sliders[number]=value;
}

void WebPage::setJoystickValue(float xpos, float ypos, float angle, float mag){
	angle-=(PI/2.0);
	joystick.xpos  = cos(angle)*mag;
	joystick.ypos  = sin(angle)*mag;
	joystick.angle = 180*angle/PI;
	joystick.mag   = mag;
}

void WebPage::setValue(String name, float data){
	for(int i=0; i<numValues; i++){
			if (values[i].used){ // compare in use slots

				if (values[i].name==name){ // check label
					if(values[i].value!=data){ // Has the value itself changed?
						//Serial.println("Update '"+name+"' "+String(data));
						values[i].value = data; // update data
						values[i].valueDirty=true;
					}
					return;
				}
			} else {
				//
				Serial.println("Create '"+name+"' "+String(i));
				values[i].used=true;
				values[i].name = name;
				values[i].value = data;
				values[i].valueDirty=true;
				values[i].labelDirty=true;
				values[i].buffer=0;
				numValuesUsed++;
				return;
			}
	}
}


bool WebPage::SendAllValues(){
	if (ws.count()==0) return false;
	if (packetBuffer) delete packetBuffer; // Deallocate old buffer
	packetBuffer = new uint8_t[numValues*8]; // Allocate a new one 8 bytes per slot max

	// Allocate float and int access arrays
	uint32_t *bufferAsInt32=(uint32_t*)packetBuffer;
	float *bufferAsFloat=(float*)packetBuffer;

	bufferAsInt32[0]=0x1e; // packet id update all


	uint32_t bufferItemCount=0; // Count the values in this update.
	for(int i=0; i<numValues; i++){
		if (values[i].used){ // Is this slot in use?
			if (values[i].valueDirty){ // Has this value changed
				values[i].valueDirty=false; //reset change flag
				bufferAsInt32[(bufferItemCount+1)*2] = i; // Index of this specific value
				bufferAsFloat[(bufferItemCount+1)*2+1] = values[i].value; // The float value
				bufferItemCount++;
			}
		}
	}

	bufferAsInt32[1]=bufferItemCount; // Record the number of elements at position 1 after the packet id
	uint32_t packetLength = (bufferItemCount+1)*8; // 8 byte header + each int/float pair


	if (bufferItemCount>0){
		// We have at least 1 item, lets send a packet.
		if ( ws.availableForWriteAll() ){ // Can we write?
			txPacketCount++;
			ws.binaryAll(packetBuffer, packetLength);
			return true; // update sent
		}
	}

	return false; // no update sent
	/*
	if(valueToSendThisLoop>=numValuesUsed)
		valueToSendThisLoop=0;
	int i= valueToSendThisLoop;
	if(values[i].used){
		if(values[i].buffer)
			delete values[i].buffer;
		values[i].buffer=new uint8_t[labelbuflen];
		sendLabelUpdate(i,values[i].buffer);
		sendValueUpdate(i,values[i].buffer);

		uint32_t datalen = 12+values[i].name.length()+1;
		for(int j=datalen;j<datalen+4;j++)values[i].buffer[j]=0;
		datalen += (4-(datalen%4)); // round up to multiple of 4
		if (datalen>=labelbuflen) datalen=labelbuflen;
		//datalen=labelbuflen;
	//		Serial.print("\r\nSending Bytes "+String(datalen)+" [");
	//		for(int j=0;j<datalen;j++){
	//
	//			Serial.print(", "+String(values[i].buffer[j])+" ");
	//
	//		}
	//		Serial.print("]");
		if (ws.availableForWriteAll())
			//txPacketCount++;
			ws.binaryAll(values[i].buffer, datalen);
		//delay(5);
		//Serial.println("Updating "+values[i].name);
	}
	valueToSendThisLoop++;
	return values[i].used;
	*/
}

bool WebPage::SendAllLabels(){
	if (ws.count()==0) return false;
	if (labelBuffer) delete labelBuffer; // remove previous buffer
	labelBuffer = new uint8_t[labelBufferSize]; // Allocate a buffer

	uint32_t *bufferAsInt32=(uint32_t*)labelBuffer;
	float *bufferAsFloat=(float*)labelBuffer;
	char *bufferAsChar=(char*)labelBuffer;

	bufferAsInt32[0]=0x1d; // packet id update all labels
	bufferAsInt32[1]=0; // num labels

	uint32_t bufferItemCount=0; // Count the values in this update.
	for(int i=0; i<numValues; i++){
		if (values[i].used){ // Is this slot in use?
			if (values[i].labelDirty){ // Has this value changed
				//Serial.println("Dirty: '"+String(values[i].name)+"' ("+String(i)+")");
				bufferAsInt32[(bufferItemCount+1)*3] = i; // Index of this specific label
				bufferAsInt32[(bufferItemCount+1)*3 + 1] = 0; // offset.
				bufferAsInt32[(bufferItemCount+1)*3 + 2] = values[i].name.length()+1; // length of this specific label. +1 for null terminator
				bufferItemCount++;
			}
		}
	}

	if (bufferItemCount==0) return false; // Nothing to update, bail.
	// Write number of labels.
	bufferAsInt32[1]=bufferItemCount;

	uint32_t startOfStringData=(bufferItemCount+1)*12;
	uint32_t stringOffset = 0;


	//Serial.println("Item Count: '"+String(bufferItemCount)+"'");
	//Serial.println("String Data Start: '"+String(startOfStringData)+"'");
	// Load changed strings into buffer.
	// update offsets as you load them in
	for(int i=0; i<bufferItemCount; i++){
		uint32_t index  = bufferAsInt32[(i+1)*3];
		uint32_t length = bufferAsInt32[(i+1)*3 + 2];
		if (startOfStringData+stringOffset+values[index].name.length() > labelBufferSize){
			//Serial.println("Strings won't fit into buffer!");
			//Serial.println("target: "+String(startOfStringData+stringOffset+values[index].name.length()));
			//Serial.println("limit:  "+String(labelBufferSize));
			return false;
		}
		//Serial.print("Processing #"+String(i)+"  len: "+String(length)+"... ");
		memcpy(&bufferAsChar[startOfStringData+stringOffset],values[index].name.c_str(),length);
		stringOffset+=length;
		values[index].labelDirty=false;
		//Serial.println("Done! packet offset: "+String(startOfStringData+stringOffset));

	}
	//Serial.println("Total Bytes in Buf: '"+String(startOfStringData+stringOffset)+"'");
	uint32_t packetLength = startOfStringData+stringOffset;
	packetLength += (4-(packetLength%4));
	if (bufferItemCount>0){
		// We have at least 1 item, lets send a packet.
		if ( ws.availableForWriteAll() ){ // Can we write?
			txPacketCount++;
			ws.binaryAll(labelBuffer,packetLength );
			return true; // update sent
		}
	}
	return false;
}

void WebPage::sendValueUpdate(uint32_t index,uint8_t *buffer){
	if(index>numValues-1) return;
//	if (!values[index].used) return;
//	if (values[index].oldValue==values[index].value) return;

	uint32_t *bufferAsInt32=(uint32_t*)buffer;
	float *bufferAsFloat=(float*)buffer;
	bufferAsInt32[0]=0x10;
	bufferAsInt32[1]=index;
	bufferAsFloat[2]=values[index].value;
}


void WebPage::sendLabelUpdate(uint32_t index,uint8_t *buffer){
	if(index>numValues-1) return;
//	if (!values[index].used) return;
//	if(!values[index].valueDirty) return;
	values[index].valueDirty=false;

	// Write out the string to the buffer. offset by 12 bytes.
	for(int i=0;i<values[index].name.length();i++){
		buffer[i+12]=values[index].name.c_str()[i];
	}
	buffer[values[index].name.length()+12]=0;


}


void WebPage::newButton(String url, void (*handler)(String), String label, String description){

}


void WebPage::valueChanged(String name, float value){
	//ESP_LOGI("WebPage::valueChanged","Got async change for '%s'",String2Chars(name)); //
	setValue(name,value);
}

void WebPage::markAllDirty(){
	for(int i=0; i<numValues; i++){
		if (values[i].used){ // Is this slot in use?
			values[i].labelDirty=true;
			values[i].valueDirty=true;
		}
	}
}


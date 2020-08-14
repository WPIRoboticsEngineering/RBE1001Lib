#include <Arduino.h>
#include <WebServer.h>
#include <HTTP_Method.h>
#include "WebPage.h"
#include "static/indexhtml.h"
#include "static/nipplejsminjs.h"
#include <ESPAsyncWebServer/ESPAsyncWebServer.h>
#include "Motor.h"


AsyncWebServer server(80);
AsyncWebSocket ws("/test");
static WebPage *thisPage;
static char stringBuffer[200];

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  uint32_t *asInt = (uint32_t *)data;
  thisPage->packetCount++;
  float    *asFloat = (float *)data;
  if(type == WS_EVT_CONNECT){
	thisPage->SendAllLabelsAndValues();
    //Serial.println("Websocket client connection received");

  } else if(type == WS_EVT_DISCONNECT){
    //Serial.println("Client disconnected");

  } else if(type == WS_EVT_DATA){
	  if (len<3) {
		  //Serial.println("Packet too short!");
		  return;
	  }
	  uint32_t command = asInt[0];
    // Data Format
    // 4B: Message Type
	//		0x10 (16)	Value Update
	//  	  	4B: value index
	//			4B: value data (float)
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
	  values[i].oldValue=0;
	  values[i].name=String("");
  }
  for(int i=0; i<numSliders; i++) sliders[i]=0;
}


void updateTask(void *param){
	int labinterval=0;
	while(1){
		labinterval++;
		for (int i=0; i<numValues; i++){

			if (labinterval<=0){
				delay(5);
				thisPage->sendLabelUpdate(i);
			}
			delay(5);
			thisPage->sendValueUpdate(i); // push async update to ui
		}
		if (labinterval<=0) labinterval=100;
		delay(60);
	}
}

void WebPage::initalize(){
	//ESP_LOGI("WebPage::WebPage","WebPage Init..");
	server.begin();
	//Serial.println("HTTP server started");
//
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", String(index_html));
    });
    server.on("/nipplejs.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/javascript", String(nipplejs_min_js));
    });

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    xTaskCreatePinnedToCore(
    		updateTask, /* Function to implement the task */
          "updateTask", /* Name of the task */
          40000,  /* Stack size in words */
          NULL,  /* Task input parameter */
          0,  /* Priority of the task */
          &thisPage->updateTaskHandle,  /* Task handle. */
          0); /* Core where the task should run */
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

						//Serial.println("Update '"+name+"' "+String(data));
						values[i].value = data; // update data

					return;
				}
			} else {
				//
				Serial.println("Create '"+name+"' "+String(i));
				values[i].used=true;
				values[i].name = name;
				values[i].value = data;

				return;
			}
	}
}


void WebPage::SendAllLabelsAndValues(){
	for(int i=0; i<numValues; i++){
		sendLabelUpdate(i);
	}
	for(int i=0; i<numValues; i++){
		sendValueUpdate(i);
	}
}

#define valbuflen 12
void IRAM_ATTR WebPage::sendValueUpdate(uint32_t index){
	if(index>numValues-1) return;
	if (!values[index].used) return;
	if (values[index].oldValue==values[index].value) return;
	values[index].oldValue=values[index].value;
	uint8_t buffer[valbuflen];
	uint32_t *bufferAsInt32=(uint32_t*)&buffer;
	float *bufferAsFloat=(float*)&buffer;
	bufferAsInt32[0]=0x10;
	bufferAsInt32[1]=index;
	bufferAsFloat[2]=values[index].value;
	if (ws.availableForWriteAll())
		ws.binaryAll(buffer, valbuflen);
}

#define labelbuflen 64
void WebPage::sendLabelUpdate(uint32_t index){
	if(index>numValues-1) return;
	if (!values[index].used) return;
	uint8_t buffer[labelbuflen];
	// clear buffer
	for (int i=0; i<labelbuflen; i++) buffer[i]=0;
	// cast as 32 bit int.
	uint32_t *bufferAsInt32=(uint32_t*)&buffer;
	bufferAsInt32[0]=0x1f; // command, label update
	bufferAsInt32[1]=index; // index
	// Write out the string to the buffer. offset by 12 bytes.
	values[index].name.toCharArray((char *)buffer+12, labelbuflen-12);
	// only send filled data
	uint32_t datalen = 12+values[index].name.length();
	datalen += (4-(datalen%4)); // round up to multiple of 4
	if (datalen>=labelbuflen) datalen=labelbuflen;

	if (ws.availableForWriteAll())
		ws.binaryAll(buffer, datalen);
}


void WebPage::newButton(String url, void (*handler)(String), String label, String description){

}


void WebPage::valueChanged(String name, float value){
	//ESP_LOGI("WebPage::valueChanged","Got async change for '%s'",String2Chars(name)); //
	setValue(name,value);
}

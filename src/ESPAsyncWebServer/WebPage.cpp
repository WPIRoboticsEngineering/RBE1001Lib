#include <Arduino.h>
#include <WebServer.h>
#include <HTTP_Method.h>
#include "WebPage.h"
#include "static/indexhtml.h"
#include "static/nipplejsminjs.h"
#include <ESPAsyncWebServer/ESPAsyncWebServer.h>



AsyncWebServer server(80);
AsyncWebSocket ws("/test");
static WebPage *thisPage;
static char stringBuffer[100];

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  uint32_t *asInt = (uint32_t *)data;
  thisPage->packetCount++;
  float    *asFloat = (float *)data;
  if(type == WS_EVT_CONNECT){
	thisPage->SendAllLabelsAndValues();
    Serial.println("Websocket client connection received");

  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");

  } else if(type == WS_EVT_DATA){
	  if (len<3) {
		  Serial.println("Packet too short!");
		  return;
	  }
	  uint32_t command = asInt[0];
    // Data Format
    // 4B: Message Type
	//		0x10	Value Update
	//  	  	4B: value index
	//			4B: value data (float)
	//		0x1f	New Value
	//			4B: value index
	//			*B: value name
    // 		0x20	Joystick Update (size 5*4 20 bytes)
    //			4B: Position X  (float, 0.0-1.0)
	//			4B: Position Y  (float, 0.0-1.0)
	//			4B: Angle       (float, radians)
	//			4B: Magnitude   (float, 0.0-1.0)
    // 		0x30	Slider Update
    //			4B: Slider Number Uint32
	//			4B: Slider Value (float, 0.0-1.0)
    //      0x40	Button Update
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
	str.toCharArray(stringBuffer, 100, 0);
	return stringBuffer;
}


WebPage::WebPage() {
  thisPage = this;
  joystick.xpos  = 0;
  joystick.ypos  = 0;
  joystick.angle = 0;
  joystick.mag   = 0;
  for(int i=0; i<numValues; i++) values[i].used=false;
  for(int i=0; i<numSliders; i++) sliders[i]=0;
}


void WebPage::initalize(){
	ESP_LOGI("WebPage::WebPage","WebPage Init..");
	server.begin();
	Serial.println("HTTP server started");
//
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", String(index_html));
    });
    server.on("/nipplejs.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/javascript", String(nipplejs_min_js));
    });

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
}

float WebPage::getSliderValue(uint32_t number){
	//Serial.println("Index:\t"+String(number));
	if (number>3) return 0.0;
	return sliders[number];
}

JoyData * WebPage::getJoystickData(){
/*
	Serial.println("Joystick Update");
	Serial.println("X pos:\t"+String(joystick.xpos));
	Serial.println("Y pos:\t"+String(joystick.ypos));
	Serial.println("Angle:\t"+String(joystick.angle));
	Serial.println("Dist:\t"+String(joystick.mag));
	*/
	return &joystick;
}

void WebPage::setSliderValue(uint32_t number, float value){
	if (number<4) thisPage->sliders[number]=value;
}

void WebPage::setJoystickValue(float xpos, float ypos, float angle, float mag){
	joystick.xpos  = xpos;
	joystick.ypos  = ypos;
	joystick.angle = angle;
	joystick.mag   = mag;
}

void WebPage::setValue(String name, float data){
	for(int i=0; i<numValues; i++){
			if (values[i].used){ // compare in use slots

				if (values[i].name==name){ // check label
					if(values[i].value!=data){ // check if data changed
						Serial.println("Update '"+name+"' "+String(data));
						values[i].value = data; // update data
						sendValueUpdate(i); // push async update to ui
					}
					return;
				}
			} else {
				//
				Serial.println("Create '"+name+"'");
				values[i].used=true;
				values[i].name = name;
				values[i].value = data;
				sendLabelUpdate(i);
				sendValueUpdate(i);
				return;
			}
	}
}

#define valbuflen 8
void WebPage::SendAllLabelsAndValues(){
	for(int i=0; i<numValues; i++){
		sendLabelUpdate(i);
	}
	for(int i=0; i<numValues; i++){
		sendValueUpdate(i);
	}
}

void WebPage::sendValueUpdate(uint32_t index){
	if(index>numValues-1) return;
	uint8_t buffer[valbuflen];
	uint32_t *bufferAsInt32=(uint32_t*)&buffer;
	float *bufferAsFloat=(float*)&buffer;
	bufferAsInt32[0]=0x10;
	bufferAsFloat[1]=values[index].value;
	if (ws.availableForWriteAll())
		ws.binaryAll(buffer, valbuflen);
}

#define labelbuflen 256
void WebPage::sendLabelUpdate(uint32_t index){
	if(index>numValues-1) return;
	uint8_t buffer[labelbuflen];
	uint32_t *bufferAsInt32=(uint32_t*)&buffer;
	bufferAsInt32[0]=0x1f;
	values[index].name.toCharArray((char *)buffer, labelbuflen, 4);
	if (ws.availableForWriteAll())
		ws.binaryAll(buffer, 256);
}


void WebPage::newButton(String url, void (*handler)(String), String label, String description){

}

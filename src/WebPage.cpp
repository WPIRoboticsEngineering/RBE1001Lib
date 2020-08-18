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

const char *strings[12] = { "Left Encoder Degrees","Left Encoder Effort","Left Encoder Degrees-sec",
		"Right Encoder Degrees","Right Encoder Effort","Right Encoder Degrees-sec" ,
				"2 Encoder Degrees","2 Encoder Effort","2 Encoder Degrees-sec" ,
				"3 Encoder Degrees","3 Encoder Effort","3 Encoder Degrees-sec"
};

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  uint32_t *asInt = (uint32_t *)data;
  thisPage->packetCount++;
  float    *asFloat = (float *)data;
  if(type == WS_EVT_CONNECT){
    //Serial.println("Websocket client connection received");
	  thisPage->SendAllLabelsAndValues();
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
	  values[i].dirty=false;
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

		//if (ws.availableForWriteAll())
		//		ws.binaryAll(buffer, 4*12);
		delay(60);
		for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++) {
			if (Motor::list[i] != NULL) {
				thisPage->valueChanged(strings[i*3],Motor::list[i]->getCurrentDegrees());
				thisPage->valueChanged(strings[i*3+1],Motor::list[i]->GetEffort());
				thisPage->valueChanged(strings[i*3+2],Motor::list[i]->getDegreesPerSecond());
			}
		}
		thisPage->valueChanged(updtime,((float)millis())/1000.0);
	}
}

void WebPage::initalize(){
	//ESP_LOGI("WebPage::WebPage","WebPage Init..");
	server.begin();
	//Serial.println("HTTP server started");
//
    server.on("/", (WebRequestMethodComposite)HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html",String(index_html) );
    });
    server.on("/nipplejs.min.js", (WebRequestMethodComposite)HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/javascript", String(nipplejs_min_js));
    });

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    xTaskCreatePinnedToCore(
    		updateTask, /* Function to implement the task */
          "updateTask", /* Name of the task */
		  8192 * 8,  /* Stack size in words */
          NULL,  /* Task input parameter */
          4,  /* Priority of the task */
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
						values[i].dirty=true;

					return;
				}
			} else {
				//
				Serial.println("Create '"+name+"' "+String(i));
				values[i].used=true;
				values[i].name = name;
				values[i].value = data;
				values[i].dirty=true;

				return;
			}
	}
}


void WebPage::SendAllLabelsAndValues(){
	for(int i=0; i<numValues; i++){
		sendLabelUpdate(i);
		delay(5);
	}
	for(int i=0; i<numValues; i++){
		sendValueUpdate(i);
		delay(5);
	}
}


void IRAM_ATTR WebPage::sendValueUpdate(uint32_t index){
	if(index>numValues-1) return;
	if (!values[index].used) return;
	if (values[index].oldValue==values[index].value) return;
	values[index].oldValue=values[index].value;

	if(values[index].labelbuffer)
			delete values[index].labelbuffer;
	values[index].labelbuffer=new uint8_t[valbuflen+1];

	uint32_t *bufferAsInt32=(uint32_t*)values[index].labelbuffer;
	float *bufferAsFloat=(float*)values[index].labelbuffer;
	bufferAsInt32[0]=0x10;
	bufferAsInt32[1]=index;
	bufferAsFloat[2]=values[index].value;
	if (ws.availableForWriteAll())
		ws.binaryAll(values[index].labelbuffer, valbuflen);
}


void WebPage::sendLabelUpdate(uint32_t index){
	if(index>numValues-1) return;
	if (!values[index].used) return;
	if(!values[index].dirty) return;
	values[index].dirty=false;

	if(values[index].buffer)
		delete values[index].buffer;
	values[index].buffer=new uint8_t[labelbuflen];

	// clear buffer
	for (int i=0; i<labelbuflen; i++) values[index].buffer[i]=0;
	// cast as 32 bit int.
	uint32_t *bufferAsInt32=(uint32_t*)values[index].buffer;
	bufferAsInt32[0]=0x1f; // command, label update
	bufferAsInt32[1]=index; // index
	// Write out the string to the buffer. offset by 12 bytes.
	values[index].name.toCharArray((char *)values[index].buffer+12, labelbuflen-12);
	// only send filled data
	uint32_t datalen = 12+values[index].name.length();
	datalen += (4-(datalen%4)); // round up to multiple of 4
	if (datalen>=labelbuflen) datalen=labelbuflen;


	if (ws.availableForWriteAll())
		ws.binaryAll(values[index].buffer, datalen);
}


void WebPage::newButton(String url, void (*handler)(String), String label, String description){

}


void WebPage::valueChanged(String name, float value){
	//ESP_LOGI("WebPage::valueChanged","Got async change for '%s'",String2Chars(name)); //
	setValue(name,value);
}

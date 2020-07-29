#include <Arduino.h>
#include "WebPage.h"
#include "SimpleWebServer.h"
#include "staticFiles.h"


static WebPage *thisPage;
static char stringBuffer[100];

char* String2Chars(String str){
	str.toCharArray(stringBuffer, 100, 0);
	return stringBuffer;
}

static void staticCallback() {
  thisPage->sendStatic();
}

static void dataCallback() {
  thisPage->sendData();
}

WebPage::WebPage(SimpleWebServer& sws): ws(sws) {

  contents = " ";
  datahead = NULL;
  buttonhead = NULL;
  thisPage = this;
}


void WebPage::initalize(){
	ESP_LOGD("WebPage::WebPage","WebPage Init..");
	ws.initialize();

	ws.registerHandler("/readData", dataCallback);
	ws.registerHandler("/", staticCallback);
}

void WebPage::handle(){
	ws.handleClient();  // handle web page requests
}


void WebPage::newButton(String url, void (*handler)(), String label, String description) {
	int count=0;
    for (ButtonMap *bv = buttonhead; bv; bv = bv->next) {
    	count++;
        if (bv->name == label) {
        	ESP_LOGD("WebPage::newButton","Updating Button '%s', Index: %d",String2Chars(label),count-1);
            bv->desc = description;
            bv->handler=handler;
            return;
        }
    }
    ESP_LOGD("WebPage::newButton","New Button '%s', Count: %d",String2Chars(label),count);
    ButtonMap *bv = new ButtonMap;
    bv->name = String(label);
    bv->desc = String(description);
    bv->next = buttonhead;
    bv->handler = handler;
    buttonhead = bv;
}

void WebPage::setValue(String name, float value) {
	int count=0;
    for (DataValues *dv = datahead; dv; dv = dv->next) {
    	count++;
        if (dv->name == name) {
        	ESP_LOGD("WebPage::setValue","Updating Value '%s', Index: %d\tValue: %f",String2Chars(name),count-1,value);
            dv->value = value;
            return;
        }
    }
    ESP_LOGD("WebPage::setValue","New Value '%s', Count: %d",String2Chars(name),count);
    DataValues *dv = new DataValues;
    dv->name = String(name);
    dv->value = value;
    dv->next = datahead;
    datahead = dv;
}

void WebPage::sendStatic() {
  String page = getStatic();
  ESP_LOGD("WebPage::sendStatic","Sending Static asset '/'");
  ws.sendHTMLResponse(page);
}

void WebPage::sendData() {
	  ESP_LOGD("WebPage::sendData","Sending Data Update");
  String page = getData();
  ws.sendHTMLResponse(page);
}

String WebPage::getStatic(){
	return String("<html><body><h1>Static!</h1></body></html>");
}

String WebPage::getData(){
	String jsonReply = "{ \"data\": [";
	for (DataValues *dv = datahead; dv; dv = dv->next) {
		jsonReply += "{";

		jsonReply += "\"name\":\"";
		jsonReply += dv->name;
		jsonReply += "\",";

		jsonReply += "\"value\":\"";
		jsonReply += String(dv->value);
		jsonReply += "\"";

		jsonReply +="}";
		if (dv->next) jsonReply += ",";
	}
	jsonReply += "],\"buttons\":[";

	for (ButtonMap *dv = buttonhead; dv; dv = dv->next) {
		jsonReply += "{";

		jsonReply += "\"name\":\"";
		jsonReply += dv->name;
		jsonReply += "\",";

		jsonReply += "\"desc\":\"";
		jsonReply += dv->desc;
		jsonReply += "\"";

		jsonReply +="}";
		if (dv->next) jsonReply += ",";
	}

	jsonReply += "] }";
	return jsonReply;
}

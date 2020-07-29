#include <Arduino.h>
#include <WebServer.h>
#include <HTTP_Method.h>
#include "WebPage.h"
#include "staticFiles.h"

static WebServer server(80);

static WebPage *thisPage;
static char stringBuffer[100];

char* String2Chars(String str){
	str.toCharArray(stringBuffer, 100, 0);
	return stringBuffer;
}

static void staticCallback() {
  thisPage->sendStatic();
}

static void valueCallback() {
  thisPage->sendValues();
}

static void buttonCallback() {
  thisPage->sendButtons();
}

static void buttonHandlerCallback(){
	if ( thisPage->handleButton(server.uri(),server.arg("value")) ){
		server.send(200, "text/plain", "ok");
	} else {
		server.send(404, "text/plain", "Not Found.");
	}
}


WebPage::WebPage() {

  contents = " ";
  datahead = NULL;
  buttonhead = NULL;
  thisPage = this;
}


void WebPage::initalize(){
	ESP_LOGI("WebPage::WebPage","WebPage Init..");
	server.begin();
	Serial.println("HTTP server started");


	server.on("/readValues", valueCallback);
	server.on("/readButtons", buttonCallback);
	server.onNotFound(buttonHandlerCallback);
	server.on("/", staticCallback);
}

bool WebPage::handleButton(String uri,String value){
	for (ButtonMap *bv = buttonhead; bv; bv = bv->next) {
		if ("/"+bv->URL == uri){
			ESP_LOGI("WebPage::handleButton","Button Click for '%s'",String2Chars(bv->name));
			bv->handler(value);
			return true;
		}
	}
	return false;
}



void WebPage::handle(){
	server.handleClient();
}


void WebPage::newButton(String url, void (*handler)(String), String label, String description) {
	int count=0;
    for (ButtonMap *bv = buttonhead; bv; bv = bv->next) {
    	count++;
        if (bv->name == label) {
        	ESP_LOGI("WebPage::newButton","Updating Button '%s', Index: %d",String2Chars(label),count-1);
            bv->desc = description;
            bv->URL = String(url);
            bv->handler=handler;
            return;
        }
    }
    ESP_LOGI("WebPage::newButton","New Button '%s', Count: %d, URL: %s",String2Chars(label),count,String2Chars(url));
    ButtonMap *bv = new ButtonMap;
    bv->name = String(label);
    bv->desc = String(description);
    bv->URL = String(url);
    bv->next = buttonhead;
    bv->handler = handler;
    buttonhead = bv;
}

void WebPage::setValue(String name, float value) {
	int count=0;
    for (DataValues *dv = datahead; dv; dv = dv->next) {
    	count++;
        if (dv->name == name) {
        	//ESP_LOGD("WebPage::setValue","Updating Value '%s', Index: %d\tValue: %f",String2Chars(name),count-1,value);
            dv->value = value;
            return;
        }
    }
    ESP_LOGI("WebPage::setValue","New Value '%s', Count: %d",String2Chars(name),count);
    DataValues *dv = new DataValues;
    dv->name = String(name);
    dv->value = value;
    dv->next = datahead;
    datahead = dv;
}

void WebPage::sendStatic() {
  String page = getStatic();
  ESP_LOGI("WebPage::sendStatic","Sending Static asset '/'");
  server.send(200, "text/plain", page);
}

void WebPage::sendValues() {
	  ESP_LOGI("WebPage::sendData","Sending Values Update");
  String page = getValues();
  server.send(200, "text/plain", page);
}

void WebPage::sendButtons() {
  ESP_LOGI("WebPage::sendData","Sending Buttons Update");
  String page = getButtons();
  server.send(200, "text/plain", page);
}

String WebPage::getStatic(){
	return String("<html><body><h1>Static!</h1></body></html>");
}

String WebPage::getButtons(){
	String jsonReply = "[";
	for (ButtonMap *dv = buttonhead; dv; dv = dv->next) {
		jsonReply += "{";

		jsonReply += "\"name\":\"";
		jsonReply += dv->name;
		jsonReply += "\",";

		jsonReply += "\"URL\":\"";
		jsonReply += dv->URL;
		jsonReply += "\",";

		jsonReply += "\"desc\":\"";
		jsonReply += dv->desc;
		jsonReply += "\"";

		jsonReply +="}";
		if (dv->next) jsonReply += ",";
	}
	jsonReply += "]";
	return jsonReply;
}

String WebPage::getValues(){
	String jsonReply = "[";
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
	jsonReply += "]";
	return jsonReply;
}

#include <WebServer.h>
#include <HTTP_Method.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Esp32WifiManager.h>

#include "SimpleWebServer.h"
WifiManager manager;
static WebServer server(80);

SimpleWebServer::SimpleWebServer() {
}

void SimpleWebServer::initialize() {
	manager.setup();
	while (manager.getState() != Connected) {
		delay(500);
		Serial.print(".");
	}
	if (MDNS.begin("esp32")) {
		Serial.println("MDNS responder started");
	}
	Serial.print("WiFi IP: ");
	Serial.println(WiFi.localIP());

	server.begin();
	Serial.println("HTTP server started");
}

void SimpleWebServer::sendResponse(const String &response) {
	server.send(200, "text/plain", response);
}

void SimpleWebServer::sendHTMLResponse(const String &response) {
	server.send(200, "text/html", response);
}

void SimpleWebServer::handleClient() {
	server.handleClient();
}

void SimpleWebServer::registerHandler(String url, void (*handler)()) {
	server.on(url, handler);
}

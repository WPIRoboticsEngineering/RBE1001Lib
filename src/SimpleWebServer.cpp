#include <WebServer.h>
#include <HTTP_Method.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "SimpleWebServer.h"

static WebServer server(80);

SimpleWebServer::SimpleWebServer() {
}

void SimpleWebServer::initialize(const char *ssid, const char *password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
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

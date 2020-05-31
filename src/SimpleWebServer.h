#pragma once
#include <Arduino.h>
class SimpleWebServer {
  public:
    void initialize();
    void handleClient();
    void registerHandler(String url, void (*handler)());
    void sendResponse(const String &response);
    void sendHTMLResponse(const String &response);
    SimpleWebServer();
};

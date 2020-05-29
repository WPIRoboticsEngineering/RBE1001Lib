#pragma once
class SimpleWebServer {
  public:
    void initialize(const char *ssid, const char *password);
    void handleClient();
    void registerHandler(String url, void (*handler)());
    void sendResponse(const String &response);
    void sendHTMLResponse(const String &response);
    SimpleWebServer();
};

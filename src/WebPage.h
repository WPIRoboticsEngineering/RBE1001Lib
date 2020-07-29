#pragma once
#include <Arduino.h>
#include "SimpleWebServer.h"

typedef struct ButtonMap {
    String name;
    String desc;
    struct ButtonMap *next;
    void (*handler)();
} ButtonMap;

typedef struct DataValues {
    String name;
    float value;
    struct DataValues *next;
} DataValues;



class WebPage {
  public:
    WebPage(SimpleWebServer& sws);
    void newButton(String url, void (*handler)(), String label, String description);
    void setValue(String name, float value);
    void sendStatic();
    void sendData();
    void handle();
    void initalize();
  private:
    String getStatic();
    String getData();
    String contents;
    SimpleWebServer ws;
    String html;
    DataValues *datahead;
    ButtonMap *buttonhead;
};

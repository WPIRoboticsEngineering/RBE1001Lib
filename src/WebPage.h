#pragma once
#include <Arduino.h>
#include "SimpleWebServer.h"

typedef struct DataValues {
    String name;
    float value;
    struct DataValues *next;
} DataValues;

class WebPage {
  public:
    WebPage(SimpleWebServer& sws);
    void newButton(String url, void (*handler)(), String label, String description);
    String getPage();
    void finishPage();
    void setValue(String name, float value);
    String getHTML();
    void sendHTML();
    void dataUpdate();
  private:
    void add(String s);
    void add(float f);
    void postResponse();
    String contents;
    SimpleWebServer ws;
    String html;
    DataValues *head;
};

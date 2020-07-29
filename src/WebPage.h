#pragma once
#include <Arduino.h>
#include <WebServer.h>

typedef struct ButtonMap {
    String name;
    String desc;
    String URL;
    struct ButtonMap *next;
    void (*handler)(String);
} ButtonMap;

typedef struct DataValues {
    String name;
    float value;
    struct DataValues *next;
} DataValues;



class WebPage {
  public:
    WebPage();
    void newButton(String url, void (*handler)(String), String label, String description);
    void setValue(String name, float value);
    void sendStatic();
    void sendValues();
    void sendButtons();
    void handle();
    bool handleButton(String uri,String value);
    void initalize();
  private:
    String getStatic();
    String getValues();
    String getButtons();
    String contents;
    String html;
    DataValues *datahead;
    ButtonMap *buttonhead;

};

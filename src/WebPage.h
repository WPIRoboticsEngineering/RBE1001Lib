#pragma once
#include <Arduino.h>
#include <WebServer.h>

typedef struct _JoyData{
	float xpos; float ypos; float angle; float mag;
} JoyData;

class WebPage {
  public:
    WebPage();
    void initalize();

    float getSliderValue(uint32_t number);
    JoyData getJoystickData();

    void setSliderValue(uint32_t number, float value);
    void setJoystickValue(float xpos, float ypos, float angle, float mag);
    void setValue(String value, float data);
    void newButton(String url, void (*handler)(String), String label, String description);
    float sliders[4];
    JoyData joystick;
    uint32_t packetCount = 0;

};

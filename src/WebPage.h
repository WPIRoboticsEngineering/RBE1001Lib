#pragma once
#include <Arduino.h>
#include <WebServer.h>

typedef struct JoyData{
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
    float sliders[4];
    JoyData joystick;


};

#pragma once
#include <Arduino.h>
#include <WebServer.h>

typedef struct _JoyData {
	float xpos;
	float ypos;
	float angle;
	float mag;
} JoyData;

typedef struct _telemetryValue {
	String name;
	float value;
	bool used;
} telemetryValue;

#define numSliders 4
#define numValues 10

class WebPage {
public:
	WebPage();
	void initalize();

	float getSliderValue(uint32_t number);
	JoyData *getJoystickData();
	float getJoystickAngle();
	float getJoystickMagnitude();
	float getJoystickX();
		float getJoystickY();
	void setSliderValue(uint32_t number, float value);
	void setJoystickValue(float xpos, float ypos, float angle, float mag);
	void setValue(String name, float data);
	void newButton(String url, void (*handler)(String), String label,
			String description);

	void SendAllLabelsAndValues();
	float sliders[numSliders];
	telemetryValue values[numValues];
	JoyData joystick;
	uint32_t packetCount = 0;
private:
	void sendValueUpdate(uint32_t index);
	void sendLabelUpdate(uint32_t index);

};

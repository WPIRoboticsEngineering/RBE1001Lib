#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "Motor.h"
#include "AsyncValueListener.h"
#define labelbuflen 256
#define valbuflen (sizeof(float)*3)
typedef struct _JoyData {
	float xpos;
	float ypos;
	float angle;
	float mag;
} JoyData;

typedef struct _telemetryValue {
	String name;
	float value;  // the value
	bool used;    // Slot in use flag
	bool valueDirty;   // Slot has new data flag
	bool labelDirty;
	uint8_t *buffer;
} telemetryValue;

#define numSliders 4
#define numValues 30
#define labelBufferSize 512

class WebPage : public AsyncValueListener {
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

	bool SendAllValues();
	bool SendAllLabels();
	float sliders[numSliders];
	telemetryValue values[numValues];
	int numValuesUsed=0;

	void valueChanged(String name, float value);


	JoyData joystick;
	uint32_t txPacketCount = 0;
	uint32_t rxPacketCount = 0;
	TaskHandle_t updateTaskHandle;
	uint32_t motor_count;
	void sendValueUpdate(uint32_t index,uint8_t *buffer);
	void sendLabelUpdate(uint32_t index,uint8_t *buffer);

	uint8_t * packetBuffer;
	uint8_t * labelBuffer;
private:
	//int valueToSendThisLoop=0;

};

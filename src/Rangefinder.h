#pragma once
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#define MAX_POSSIBLE_INTERRUPT_RANGEFINDER 4

/**
 *  \brief Rangefinder objects wrap a FreeRTOS thread with pin change interrupts to read trigger/echo style ultrasonic sensors
 *
 *  Rangefinder objects can be declared statically
 *
 *  Range is calculated continuously and the current measurement is retrieved with getDistanceCM()
 *
 *  Many objects can be declared, and they are pinged one at a time in a round-robbin configuration to prevent echo and cross-talk.
 *
 *  When pins are attached, the thread is started. The thread checks for timeouts and re sets the round-robbin.
 */
class Rangefinder {
public:
	Rangefinder();
	void attach(int trigger, int echo);
	static hw_timer_t *timer;

	static int numberOfFinders;
	static bool timoutThreadStarted;
	static int pingIndex;
	static bool forceFire;
	portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;
	int echoPin;
	int triggerPin;
	volatile unsigned long startTime;
	volatile unsigned long roundTripTime;
	static Rangefinder * list[MAX_POSSIBLE_INTERRUPT_RANGEFINDER];
	/**
	 * \brief get the distance of an object from the sensor in centimeters
	 *
	 * @return the distance in centimeters
	 */
	float getDistanceCM();

	static void checkTimeout();
	static void fire();
	void sensorISR();

	static int getTimeoutState();

};

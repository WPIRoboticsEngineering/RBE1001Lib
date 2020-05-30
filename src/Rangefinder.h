#pragma once

#define MAX_POSSIBLE_INTERRUPT_RANGEFINDER 4

class Rangefinder {
public:
	Rangefinder(int trigger, int echo);
	static hw_timer_t *timer;

	static int numberOfFinders;
	static int timerNumber;
	static int pingIndex;
	portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;
	int echoPin;
	int triggerPin;
	volatile unsigned long startTime;
	volatile unsigned long roundTripTime;
	static Rangefinder * list[MAX_POSSIBLE_INTERRUPT_RANGEFINDER];
	/**
	 * Returns the dinstance in centimeters
	 */
	float getDistanceCM();
	/**
	 * allocateTimer
	 * @param a timer number 0-3 indicating which timer to allocate in this library
	 */
	static void allocateTimer(int timerNumber);
	static void fire();
	void sensorISR();

};

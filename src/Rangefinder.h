#pragma once

#define MAX_POSSIBLE_INTERRUPT_RANGEFINDER 4

class Rangefinder {
public:
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
	Rangefinder(int trigger, int echo);
	float getDistanceCM();
	static void allocateTimer(int timerNumber);
	void sensorISR();
};

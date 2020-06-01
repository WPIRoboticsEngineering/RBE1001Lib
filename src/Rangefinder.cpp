#include "Arduino.h"
#include "Rangefinder.h"

static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

hw_timer_t *Rangefinder::timer = NULL;
Rangefinder * Rangefinder::list[MAX_POSSIBLE_INTERRUPT_RANGEFINDER] = { NULL, };
int Rangefinder::numberOfFinders = 0;
int Rangefinder::timerNumber = -1;
int Rangefinder::pingIndex = 0;
/*
 * The procedure is to send a 10us pulse on the trigger line to
 * cause the ultrasonic burst to be sent out the speaker. When
 * that happens, the echo line will go high. When the echo from
 * the sound returns, the echo line will go low. This pulse length
 * is measured using the pulseIn function that returns the round-trip
 * time in microseconds which is then converted to centimeters and
 * returned.
 * The timer interrupt routine will send the 10us pulse every 100ms and
 * the sensor ISR will time from the rising edge to the falling edge.
 * This time is saved in "roundTripTime" and used to compute the distance.
 */
void IRAM_ATTR onTimer() {
	portENTER_CRITICAL_ISR(&timerMux);
	Rangefinder::fire();
	portEXIT_CRITICAL_ISR(&timerMux);
}
/**
 * allocateTimer
 * @param a timer number 0-3 indicating which timer to allocate in this library
 */
void Rangefinder::allocateTimer(int timerNumber) {
	if (Rangefinder::timerNumber < 0) {
		Rangefinder::timerNumber = timerNumber;
		timer = timerBegin(Rangefinder::timerNumber, 80, true);
		timerAttachInterrupt(timer, &onTimer, true);
		timerAlarmWrite(timer, 100000, true);
		timerAlarmEnable(timer);
	}
}
void IRAM_ATTR sensorISR0() {
	Rangefinder::list[0]->sensorISR();
}
void IRAM_ATTR sensorISR1() {
	Rangefinder::list[1]->sensorISR();
}
void IRAM_ATTR sensorISR2() {
	Rangefinder::list[2]->sensorISR();
}
void IRAM_ATTR sensorISR3() {
	Rangefinder::list[3]->sensorISR();
}
void Rangefinder::fire() {
	if (Rangefinder::numberOfFinders > 0) {
		// round robin all of the sensors to prevent cross talk
		Rangefinder::pingIndex++;
		if (Rangefinder::pingIndex == Rangefinder::numberOfFinders)
			Rangefinder::pingIndex = 0;
		digitalWrite(Rangefinder::list[Rangefinder::pingIndex]->triggerPin,
		HIGH); // 10us pulse to start the process
		delayMicroseconds(10);
		digitalWrite(Rangefinder::list[Rangefinder::pingIndex]->triggerPin,
				LOW);
	}

}
void Rangefinder::sensorISR() {
	portENTER_CRITICAL(&synch);
	if (digitalRead(echoPin)) {
		startTime = micros();
	} else {
		roundTripTime = micros() - startTime;
		fire();
		timerWrite(timer, 0); // clear the timeout timer since the ping came back
	}
	portEXIT_CRITICAL(&synch);
}

/*
 * Initialize the rangefinder with the trigger pin and echo pin
 * numbers.
 */
Rangefinder::Rangefinder(int trigger, int echo) {
	triggerPin = trigger;
	echoPin = echo;
	pinMode(triggerPin, OUTPUT);
	pinMode(echoPin, INPUT);

	for (int i = 0; i < MAX_POSSIBLE_INTERRUPT_RANGEFINDER; i++) {
		if (Rangefinder::list[i] == NULL) {
			digitalWrite(triggerPin, LOW); // be sure to start from low
			delayMicroseconds(2);
			Rangefinder::list[i] = this;
			Rangefinder::numberOfFinders++;
			switch (i) {
			case 0:
				attachInterrupt(digitalPinToInterrupt(echoPin), sensorISR0,
				CHANGE);
				break;
			case 1:
				attachInterrupt(digitalPinToInterrupt(echoPin), sensorISR1,
				CHANGE);
				break;
			case 2:
				attachInterrupt(digitalPinToInterrupt(echoPin), sensorISR2,
				CHANGE);
				break;
			case 3:
				attachInterrupt(digitalPinToInterrupt(echoPin), sensorISR3,
				CHANGE);
				break;
			}
			return;
		}
	}
	Serial.println("FAULT too many range finders!");
	while (1)
		;
}

/*
 * Get the distance from the rangefinder
 */
float Rangefinder::getDistanceCM() {
	float distance;
	portENTER_CRITICAL(&synch);
	distance = (roundTripTime * 0.0343) / 2.0;
	portEXIT_CRITICAL(&synch);
	return distance;
}

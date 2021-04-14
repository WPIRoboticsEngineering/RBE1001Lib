#include "Rangefinder.h"
#include "Timer.h"

static TaskHandle_t complexHandlerTaskUS;

Rangefinder * Rangefinder::list[MAX_POSSIBLE_INTERRUPT_RANGEFINDER] = { NULL, };
int Rangefinder::numberOfFinders = 0;
bool Rangefinder::timoutThreadStarted = false;
bool Rangefinder::forceFire = false;
int Rangefinder::pingIndex = 0;

static long threadTimeout;
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
void onTimer(void *param) {
	Serial.println("Starting the Ultrasonic loop thread");
	threadTimeout = millis();
	while (1) {
		vTaskDelay(10); //sleep 10ms
		if (Rangefinder::forceFire)
			Rangefinder::fire();
		else
			Rangefinder::checkTimeout();
	}
	Serial.println("ERROR Pid thread died!");

}


int Rangefinder::getTimeoutState() {
	return millis() - threadTimeout;
}

void Rangefinder::checkTimeout() {
	// check to see if an ultrasonic timed out
	bool run = false;
	run = Rangefinder::getTimeoutState() > 100;
	if (run) {
		//Serial.println("Ultrasonic thread timeout!");
		fire();
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
	threadTimeout = millis();
	forceFire = false;
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
		forceFire = true;
	}
	portEXIT_CRITICAL(&synch);
}
void Rangefinder::attach(int trigger, int echo) {
	triggerPin = trigger;
	echoPin = echo;
	pinMode(triggerPin, OUTPUT);
	pinMode(echoPin, INPUT);
	if (Rangefinder::timoutThreadStarted == false) {
		Serial.println("Spawing rangefinder timeout thread");
		Rangefinder::timoutThreadStarted = true;
		xTaskCreatePinnedToCore(onTimer, "Rangefinder Thread", 8192, NULL, 2, // low priority timout thread
				&complexHandlerTaskUS, 0);
	}
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
	while(roundTripTime<0){
		delay(1);
	}

}
/*
 * Initialize the rangefinder with the trigger pin and echo pin
 * numbers.
 */
Rangefinder::Rangefinder() {
}
/**
 * \brief get the time of latest round trip in microseconds
 *
 * @return the time in microseconds
 */
long Rangefinder::getRoundTripTimeMicroSeconds() {
	long time;
	portENTER_CRITICAL(&synch);
	time=roundTripTime;
	portEXIT_CRITICAL(&synch);
	return time;
}
/*
 * Get the distance from the rangefinder
 */
float Rangefinder::getDistanceCM() {
	float distance;
	distance = (getRoundTripTimeMicroSeconds() * 0.0343) / 2.0;
	return distance;
}

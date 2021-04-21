#pragma once
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#define MAX_POSSIBLE_INTERRUPT_RANGEFINDER 10

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
 *
 *  This library supports up to 4 range finders
 */
class Rangefinder {
public:
	Rangefinder();
	/**
	 *  \brief Attach 2 pins to be used as triger and echo
	 *
	 *  The trigger pin needs to be an output, and the echo pin needs to have CHANGE interrupts.
	 *
	 *  Interrupts are managed through Arduio attachInterrupt()
	 */
	void attach(int trigger, int echo);
	/**
	 * running count of attached range finders
	 */
	static int numberOfFinders;
	/**
	 * flag to keep track of the state of the thread, started or not
	 */
	static bool timoutThreadStarted;
	/**
	 * which index in the round robbin to ping
	 */
	static int pingIndex;
	/**
	 * flag to force a fire from the timeout thread
	 */
	static bool forceFire;
	/**
	 * synchronization object to lock out access to volitile memory
	 */
	portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;
	/**
	 * GPIO pin number for the echo
	 */
	int echoPin;
	/**
	 * GPIO pin for the trigger
	 */
	int triggerPin;
	/**
	 * Time the pulse started
	 */
	volatile unsigned long startTime;
	/**
	 * the cached time of the latest sensor read.
	 */
	volatile unsigned long roundTripTime=-1;
	/**
	 * \brief get the time of latest round trip in microseconds
	 *
	 * @return the time in microseconds
	 */
	long getRoundTripTimeMicroSeconds();
	/**
	 * \brief list of attached rangefinders
	 *
	 * when attach completes, a pointer to the object being attahced is added to this list.
	 * THis list is read from the timeout thread and Fire in order to run the round-robbin of all sensors.
	 */
	static Rangefinder * list[MAX_POSSIBLE_INTERRUPT_RANGEFINDER];
	/**
	 * \brief get the distance of an object from the sensor in centimeters
	 *
	 * @return the distance in centimeters
	 */
	float getDistanceCM();
	/**
	 * \brief check the current state of timeout and fire if its time to do so
	 */
	static void checkTimeout();
	/**
	 * \brief fire a strobe of the trig pin
	 *
	 * this will initiate a chirp and wait for the echo to come back
	 */
	static void fire();
	/**
	 *  \brief The method called from the ISR indicating the echo pin changed state
	 *
	 *  This method is called by the static method for the associated interrupt.
	 */
	void sensorISR();
	/**
	 * \brief Function used by the timeout check thread to determine if this object has timed out
	 */
	static int getTimeoutState();


};

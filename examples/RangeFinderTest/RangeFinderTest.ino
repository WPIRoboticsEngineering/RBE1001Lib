#include <Arduino.h>
#include <RBE1001Lib.h>
#include "Rangefinder.h"

Rangefinder rangefinder1(FORWARD_ULTRASONIC_TRIG, FORWARD_ULTRASONIC_ECHO);
Rangefinder rangefinder2(SIDE_ULTRASONIC_TRIG, SIDE_ULTRASONIC_ECHO);

/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 * In this example, it sets the Serial Console speed, initializes the web server,
 * sets up some web page buttons, resets some timers, and sets the initial state
 * the robot should start in
 */
void setup() {
  // This will initialize the Serial as 115200 for prints and passwords
  Serial.begin(115200);
  // Allocate timer 0 for the rangefinder
  Rangefinder::allocateTimer(0);

}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started. In here we run the state machine, update the
 * dashboard data, and handle any web server requests.
 */
void loop() {
	delay(200);
	Serial.println("Range 1 "+String(rangefinder1.getDistanceCM())+
			" Range 2 "+String(rangefinder2.getDistanceCM()));
 }


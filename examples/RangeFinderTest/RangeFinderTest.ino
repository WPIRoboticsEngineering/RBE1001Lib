#include <Arduino.h>
#include <RBE1001Lib.h>
#include "Rangefinder.h"
Rangefinder rangefinder1;

/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 */
void setup() {
  // This will initialize the Serial as 115200 for prints
  Serial.begin(115200);
  Serial.println("Allocating Rangefinder resources");
  // Allocate timer 3 for the rangefinder
  Serial.println("Starting...");
  rangefinder1.attach(SIDE_ULTRASONIC_TRIG, SIDE_ULTRASONIC_ECHO);
}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() {
	delay(200);
	Serial.println("Range 1 "+String(rangefinder1.getDistanceCM())+" T-O: "+String(Rangefinder::getTimeoutState()));
 }


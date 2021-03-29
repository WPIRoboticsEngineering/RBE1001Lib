#include <Arduino.h>
#include <RBE1001Lib.h>

// pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
const int buttonPin = BOOT_FLAG_PIN;

LeftMotor left_motor;
RightMotor right_motor;
bool upDown = false;
/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 */
void setup()
{
	// This will initialize the Serial as 115200 for prints
	Serial.begin(115200);
}

/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop()
{
	while (digitalRead(buttonPin))
	{
	} //wait for button press
	left_motor.setSpeed(120);

	uint32_t startTime = millis();

	while (millis() - startTime < 5000) //run for 5 seconds
	{
		Serial.print(left_motor.getDegreesPerSecond());
		Serial.print('\t');
		Serial.print(left_motor.getEffortPercent());
		Serial.print('\n');

		delay(20);
	}

	left_motor.setSpeed(0);
}

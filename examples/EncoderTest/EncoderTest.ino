#include <RBE1001Lib.h>

LeftMotor left_motor;
RightMotor right_motor;

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
	Serial.print("L: ");
	Serial.print(left_motor.getCurrentDegrees());
	Serial.print('\t');

	Serial.print("R: ");
	Serial.print(right_motor.getCurrentDegrees());
	Serial.print('\n');

	delay(100);
}

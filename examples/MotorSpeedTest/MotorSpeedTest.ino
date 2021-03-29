
#include <RBE1001Lib.h>

LeftMotor left_motor;
LeftMotor right_motor;

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
	float speed = 360 * sin(2 * 3.14 * (millis() % 10000) / 10000.);
	left_motor.setSpeed(speed);
	right_motor.setSpeed(speed);

	Serial.print("SP: ");
	Serial.print(speed);
	Serial.print('\t');

	Serial.print("L: ");
	Serial.print(left_motor.getDegreesPerSecond());
	Serial.print('\t');

	Serial.print("E: ");
	Serial.print(left_motor.getEffortPercent());
	Serial.print('\n');

	delay(100);
}

#include <RBE1001Lib.h>
#include <ESP32Servo.h>
#include <ESP32AnalogRead.h>

// https://wpiroboticsengineering.github.io/RBE1001Lib/classServo.html
Servo lifter;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classESP32AnalogRead.html
ESP32AnalogRead servoPositionFeedback;


void setup() 
{
	Serial.begin(115200);
	// pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
	lifter.attach(SERVO_PIN);
	servoPositionFeedback.attach(SERVO_FEEDBACK_SENSOR);
	lifter.write(0);
}

/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started. 
 */

void loop() 
{
	uint16_t angle = (millis()/20) % 180;
	lifter.write(angle);
	Serial.println(angle);
	delay(10);
}

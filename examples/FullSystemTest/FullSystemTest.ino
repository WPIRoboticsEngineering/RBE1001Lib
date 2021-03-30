#include <RBE1001Lib.h>
#include "Motor.h"
#include "Rangefinder.h"
#include <ESP32Servo.h>
#include <ESP32AnalogRead.h>
// https://wpiroboticsengineering.github.io/RBE1001Lib/classMotor.html
LeftMotor left_motor;
RightMotor right_motor;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classRangefinder.html
Rangefinder rangefinder1;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classServo.html
Servo lifter;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classESP32AnalogRead.html
ESP32AnalogRead leftLineSensor;
ESP32AnalogRead rightLineSensor;
ESP32AnalogRead servoPositionFeedback;
bool upDown = false;
/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 */
void setup()
{
	// This will initialize the Serial as 115200 for prints
	Serial.begin(115200);
	// pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
	rangefinder1.attach(SIDE_ULTRASONIC_TRIG, SIDE_ULTRASONIC_ECHO);
	lifter.attach(SERVO_PIN);
	leftLineSensor.attach(LEFT_LINE_SENSE);
	rightLineSensor.attach(RIGHT_LINE_SENSE);
	servoPositionFeedback.attach(SERVO_FEEDBACK_SENSOR);
}

/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop()
{
	upDown = !upDown;
	int loopTime = 4000; // 4 second loop
	int servoRange = 180;

	right_motor.setSetpointWithSinusoidalInterpolation(upDown ? 360 : 0, loopTime);
	left_motor.setSetpointWithLinearInterpolation(upDown ? 360 : 0, loopTime);

	for (int i = 0; i < servoRange; i++)
	{
		delay(loopTime / servoRange);
		Serial.println("\n");
		Serial.print("Range 1 " + String(rangefinder1.getDistanceCM()));
		//		Serial.print("Speed 1 "+String(left_motor.getDegreesPerSecond())+
		//					" Speed 2 "+String(right_motor.getDegreesPerSecond()));
		Serial.print("\tCount 1 " + String(left_motor.getCurrentDegrees()) +
					 "\t Count 2 " + String(right_motor.getCurrentDegrees()));
		Serial.print("\t Line left " + String(leftLineSensor.readVoltage()));

		Serial.print("\t Line right " + String(rightLineSensor.readVoltage()));
		Serial.print("\t Servo Read " + String(servoPositionFeedback.readVoltage()));
		Serial.print("\t Servo write " + String(i));
		lifter.write(i);
	}
}

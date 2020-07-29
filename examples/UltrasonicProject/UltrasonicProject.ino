#include "ESP32Encoder.h"
#include "ESP32Servo.h"
#include "RBE1001Lib.h"
#include <Arduino.h>

Motor motor1;
Motor motor2;
Rangefinder rangeFinder; //2200 ohm (red-red-red)

void setup() {
	Serial.begin(115200);
	Motor::allocateTimer(0);
	motor1.attach(MOTOR1_PWM, MOTOR1_DIR, MOTOR1_ENCA, MOTOR1_ENCB);
	motor2.attach(MOTOR2_PWM, MOTOR2_DIR, MOTOR2_ENCA, MOTOR2_ENCB);
// 17=trigger 16=echo (thru resistor)
	rangeFinder.attach(SIDE_ULTRASONIC_TRIG, SIDE_ULTRASONIC_ECHO);
}

void setEfforts(float left, float right) {
	motor1.SetEffort(left);
	motor2.SetEffort(right);
}

void setEfforts(float both) {
	setEfforts(both, both);
}

void loop() {
	float Kp = 0.07;
	float setPoint = 28; // 11" in CM
	float actualDistance = rangeFinder.getDistanceCM();
	float error = actualDistance - setPoint;
	setEfforts(error * Kp);
	delay(10);
}

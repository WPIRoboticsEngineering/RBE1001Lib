/*
 * Motor.h
 *
 *  Created on: May 31, 2020
 *      Author: hephaestus
 */

#ifndef LIBRARIES_RBE1001LIB_SRC_MOTOR_H_
#define LIBRARIES_RBE1001LIB_SRC_MOTOR_H_
#include <ESP32Servo.h>
#include <ESP32Encoder.h>

class Motor {
private:
	ESP32PWM pwm;
	ESP32Encoder encoder;
public:
	Motor();
	virtual ~Motor();

	/**
	 * Attach the motors hardware
	 * @param MotorPWMPin the pin that produce PWM at 20kHz (Max is 250khz per DRV8838 datasheet)
	 * @param MotorDirectionPin motor direction setting pin
	 * @param the A channel of the encoder
	 * @param the B channel of the encoder
	 */
	void attach(int MotorPWMPin, int MotorDirectionPin,int EncoderA, int EncoderB );
	/*
	 * effort of the motor
	 * @param a value from -1 to 1 representing effort
	 *        0 is brake
	 *        1 is full speed clockwise
	 *        -1 is full speed counter clockwise
	 */
	void SetEffort(float effort);

	/**
	 * getDegreesPerSecond
	 *
	 * This function returns the current speed of the motor
	 *
	 * @return the speed of the motor in degrees per second
	 */
	float getDegreesPerSecond();

};

#endif /* LIBRARIES_RBE1001LIB_SRC_MOTOR_H_ */

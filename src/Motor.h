/*
 * Motor.h
 *
 *  Created on: May 31, 2020
 *      Author: hephaestus
 */

#ifndef LIBRARIES_RBE1001LIB_SRC_MOTOR_H_
#define LIBRARIES_RBE1001LIB_SRC_MOTOR_H_

class Motor {
public:
	Motor(int MotorPWMPin, int MotorDirectionPin,int EncoderA, int EncoderB );
	virtual ~Motor();
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

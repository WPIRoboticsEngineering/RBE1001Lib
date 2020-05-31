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
#define MAX_POSSIBLE_MOTORS 4
class Motor {
private:
	ESP32PWM * pwm;
	ESP32Encoder * encoder;
	int MotorPWMPin=-1;
	int directionFlag=-1;
	int interruptCountForVelocity = 0;
	int prevousCount=0;
	float cachedSpeed=0;
	int nowEncoder=0;
	int Setpoint=0;
	float kP=0.01;
	float kI=0;
	float kD=0;
public:
	//Static section
	static bool timersAllocated;
	static Motor * list[MAX_POSSIBLE_MOTORS];
	static hw_timer_t *timer;
	/**
	 * @param PWMgenerationTimer the timer to be used to generate the 20khx PWM
	 * @param controllerTimer a timer running at 1khz for PID, velocity measurment, and trajectory planning
	 */
	static void allocateTimer(int PWMgenerationTimer);
	// Class methods
	Motor();
	virtual ~Motor();

	/**
	 * Attach the motors hardware
	 * @param MotorPWMPin the pin that produce PWM at 20kHz (Max is 250khz per DRV8838 datasheet)
	 * @param MotorDirectionPin motor direction setting pin
	 * @param the A channel of the encoder
	 * @param the B channel of the encoder
	 * @note this must only be called after timers are allocated via Motor::allocateTimers
	 *
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
	/**
	 * getTicks
	 *
	 * This function returns the current count of encoders
	 * @return count
	 */
	int getCurrentTicks();
	/**
	 * Loop function
	 * this method is called by the timer to run the PID control of the motors and ensure strict timing
	 *
	 */
	void loop();
};

#endif /* LIBRARIES_RBE1001LIB_SRC_MOTOR_H_ */

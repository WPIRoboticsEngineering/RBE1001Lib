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
#include <Arduino.h>
#define MAX_POSSIBLE_MOTORS 4
#define ENCODER_CPR 12.0f
#define GEAR_BOX_RATIO 120.0f
#define QUADRATUE_MULTIPLYER 1.0f
#define TICKS_TO_DEGREES (QUADRATUE_MULTIPLYER/(ENCODER_CPR*GEAR_BOX_RATIO/360.0))
#define I_TERM_SIZE 60.0f
enum interpolateMode {
	LINEAR_INTERPOLATION, SINUSOIDAL_INTERPOLATION, VELOCITY_MODE
};
/** \brief A PID Motor class using threads, ESP32Encoder and ESP32PWM
 *
 * This Motor class is intended to be used by RBE 1001 in the WPI Robotics Department.
 *
 */
class Motor {
private:
	ESP32PWM * pwm;
	ESP32Encoder * encoder;
	int MotorPWMPin = -1;
	int directionFlag = -1;
	int interruptCountForVelocity = 0;
	int prevousCount = 0;
	float cachedSpeed = 0;

	float Setpoint = 0;
	float kP = 0.01;
	float kI = 0;
	float kD = 0;
	float runntingITerm = 0;
	/*
	 * effort of the motor
	 * @param a value from -1 to 1 representing effort
	 *        0 is brake
	 *        1 is full speed clockwise
	 *        -1 is full speed counter clockwise
	 */
	void SetEffortLocal(float effort);
	bool closedLoopControl = true;
	float currentEffort = 0;
	float duration = 0;
	float startTime = 0;
	float endSetpoint = 0;
	float startSetpoint = 0;
	/**
	 * Duration of the interpolation mode, 1 equals done, 0 starting
	 */
	float unitDuration = 1;
	float getInterpolationUnitIncrement();
	/**
	 * Current interpolation mode
	 */
	interpolateMode mode = LINEAR_INTERPOLATION;
	float milisecondPosIncrementForVelocity;

public:
	int64_t nowEncoder = 0;
	//Static section
	static bool timersAllocated;
	static Motor * list[MAX_POSSIBLE_MOTORS];
	static hw_timer_t *timer;
	/**
	 * @param PWMgenerationTimer the timer to be used to generate the 20khz PWM
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
	void attach(int MotorPWMPin, int MotorDirectionPin, int EncoderA,
			int EncoderB);
	/*
	 * effort of the motor
	 * @param a value from -1 to 1 representing effort
	 *        0 is brake
	 *        1 is full speed clockwise
	 *        -1 is full speed counter clockwise
	 */
	void SetEffort(float effort);
	/*
	 * effort of the motor
	 * @param a value from -100 to 100 representing effort
	 *        0 is brake
	 *        100 is full speed clockwise
	 *        -100 is full speed counter clockwise
	 */
	void SetEffortPercent(float percent) {
		SetEffort(percent * 0.01);
	}
	/*
	 * effort of the motor
	 * @return a value from -1 to 1 representing effort
	 *        0 is brake
	 *        1 is full speed clockwise
	 *        -1 is full speed counter clockwise
	 */
	float GetEffort();
	/*
	 * effort of the motor
	 * @return a value from -100 to 100 representing effort
	 *        0 is brake
	 *        100 is full speed clockwise
	 *        -100 is full speed counter clockwise
	 */
	float GetEffortPercent() {
		return GetEffort() * 100;
	}
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
	float getCurrentDegrees();
	/**
	 * Loop function
	 * this method is called by the timer to run the PID control of the motors and ensure strict timing
	 *
	 */
	void loop();
	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param miliseconds the number of miliseconds to get from current position to the new setpoint
	 * param mode the interpolation mode
	 */
	void SetSetpointWithTime(float newTargetInDegrees, long miliseconds,
			interpolateMode mode);
	/**
	 * SetSpeed in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newDegreesPerSecond the new speed in degrees per second
	 */
	void SetSpeed(float newDegreesPerSecond);
	/**
	 * SetSpeed in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newDegreesPerSecond the new speed in degrees per second
	 * @param miliseconds the number of miliseconds to run for
	 */
	void SetSpeed(float newDegreesPerSecond, long miliseconds);
	/**
	 * MoveTo in degrees with speed
	 * Set the setpoint for the motor in degrees and the speed you want to get there
	 * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param speedDegPerSec  is the speed in degrees per second
	*/
	void MoveTo(float newTargetInDegrees, float speedDegPerSec);
    /**
     * MoveFor a relative amount in degrees with speed
     * Set the setpoint for the motor in degrees and the speed you want to get there
     * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
     * @param deltaTargetInDegrees the new relative setpoint for the closed loop controller
     * @param speedDegPerSec  is the speed in degrees per second
    */
    void MoveFor(float deltaTargetInDegrees, float speedDegPerSec);
    
    /**
     * StartMoveFor a relative amount in degrees with speed
     * Set the setpoint for the motor in degrees and the speed you want to get there
     * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
     * @param deltaTargetInDegrees the new relative setpoint for the closed loop controller
     * @param speedDegPerSec  is the speed in degrees per second
    */
    void StartMoveFor(float deltaTargetInDegrees, float speedDegPerSec);
    
	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 */
	void SetSetpoint(float newTargetInDegrees) {
		SetSetpointWithTime(newTargetInDegrees, 0, LINEAR_INTERPOLATION);
	}
	
	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param miliseconds the number of miliseconds to get from current position to the new setpoint
	 * use linear interoplation
	 */
	void SetSetpointWithLinearInterpolation(float newTargetInDegrees,
			long miliseconds) {
		SetSetpointWithTime(newTargetInDegrees, miliseconds,
				LINEAR_INTERPOLATION);
	}

	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param miliseconds the number of miliseconds to get from current position to the new setpoint
	 * use sinusoidal interpolation
	 */
	void SetSetpointWithSinusoidalInterpolation(float newTargetInDegrees,
			long miliseconds) {
		SetSetpointWithTime(newTargetInDegrees, miliseconds,
				SINUSOIDAL_INTERPOLATION);
	}
	/**
	 * PID gains for the PID controller
	 */
	void setGains(float p, float i, float d);
};

#endif /* LIBRARIES_RBE1001LIB_SRC_MOTOR_H_ */

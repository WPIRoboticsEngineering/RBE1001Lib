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
#define TICKS_TO_DEGREES ((QUADRATUE_MULTIPLYER / (ENCODER_CPR * GEAR_BOX_RATIO / 360.0)) * -1)
#define I_TERM_SIZE 120.0f

const float DELTA_EFFORT = 0.0025;

enum interpolateMode
{
	LINEAR_INTERPOLATION = 1,
	SINUSOIDAL_INTERPOLATION = 2,
	VELOCITY_MODE = 3,
	BEZIER = 4,
	TRAPEZOIDAL = 5
};
/** \brief A PID Motor class using FreeRTOS threads, ESP32Encoder and ESP32PWM
 *
 * This Motor class is intended to be used by RBE 1001 in the WPI Robotics Department.
 *
 * Motor objects can be instantiated statically. The attach method must be called before using the motor.
 *
 * The motor uses one timer for the ESP32PWM objects. That means the static method
 *
 * Motor::allocateTimer (int PWMgenerationTimer)
 *
 * must be called before any motor objects can be attached. This method will also start the PID thread.
 *
 */
class Motor
{
private:
	/**
	 * the object that produces PWM for motor speed
	 */
	ESP32PWM *pwm;
	/**
	 * the object that keeps track of the motors position
	 */
	ESP32Encoder *encoder;
	/**
	 * True if the motor has been attached
	 */
	bool isAttached = false;
	/**
	 * an internal counter that counts iterations of the PID loop
	 * this is used to calculate 50ms timing for calculation of the velocity
	 */
	int interruptCountForVelocity = 0;
	/*
	 * this stores the previous count of the encoder last time the velocity was calculated
	 */
	int prevousCount = 0;
	/**
	 * this variable is the most recently calculated speed
	 */
	float cachedSpeed = 0;
	/**
	 * PID controller setpoint in encoder ticks
	 */
	float setpoint = 0;
	/**
	 * PID controller proportional constant
	 */
	float kP = 0.02;
	/**
	 * PID controller integral constant
	 */
	float kI = 0.01;
	/**
	 * PID controller derivitive constant
	 */
	float kD = 0;
	/**
	 * a variable to store the running avarage for the integral term
	 */
	float runntingITerm = 0;
	/*
	 * effort of the motor
	 * @param a value from -1 to 1 representing effort
	 *        0 is brake
	 *        1 is full speed clockwise
	 *        -1 is full speed counter clockwise
	 * @note this should only be called from the PID thread
	 */
	void setEffortLocal(float effort);
	/**
	 * @param PWMgenerationTimer the timer to be used to generate the 20khz PWM
	 */
	static void allocateTimer(int PWMgenerationTimer);
	/**
	 * this is a flag to switch between using the PID controller, or allowing the user to set effort 'directly'
	 *
	 */
	bool closedLoopControl = true;
	/**
	 * variable for storing target effort to allow for smoother accelerations
	 */
	float targetEffort = 0;
	/**
	 * variable for caching the current effort being sent to the PWM/direction pins
	 */
	float currentEffort = 0;
	/**
	 * PID controller Interpolation duration in miliseconds
	 */
	float duration = 0;
	/**
	 * PID controller Interpolation time in miliseconds that the interplation began
	 */
	float startTime = 0;
	/**
	 * PID controller Interpolation setpoint for the interpolation to arrive at
	 */
	float endSetpoint = 0;
	/**
	 * PID controller Interpolation setpoint at the start of interpolation
	 */
	float startSetpoint = 0;
	/**
	 * Duration of the interpolation mode, 1 equals done, 0 starting
	 */
	float unitDuration = 1;
	/**
	 * Current interpolation mode, linear, sinusoidal or velocity
	 */
	interpolateMode mode = LINEAR_INTERPOLATION;

	/**
	 * when using Red Queen mode for velocity interpolation, this is the amount of setpoint to add to the current  setpoint
	 * every milisecond to maintain a smooth velocity trajectory.
	 */
	float milisecondPosIncrementForVelocity;
	/**
	 * \brief BEZIER Control Point 0
	 *
	 * https://stackoverflow.com/a/43071667
	 */
	float BEZIER_P0 = 0.25;
	/**
	 * \brief  BEZIER Control Point 1
	 *
	 * https://stackoverflow.com/a/43071667
	 */
	float BEZIER_P1 = 0.75;

	/**
	 * \brief the amount of time to ramp up and ramp down the speed
	 */
	float TRAPEZOIDAL_time = 0;

public:
	/**
	 * GPIO pin number of the motor PWM pin
	 */
	int MotorPWMPin = -1;
	/**
	 * GPIO pin number of the motor direction output flag
	 */
	int directionFlag = -1;
	/**
	 * GPIO pin number of the motor encoder A
	 */
	int MotorEncAPin = -1;
	/**
	 * GPIO pin number of the motor encoder B
	 */
	int MotorEncBPin = -1;
	/**
	 * use the internal state and current time to comput where along the path from start to finish the interpolation is
	 */
	float getInterpolationUnitIncrement();
	/**
	 * Variable to store the latest encoder read from the encoder hardware as read by the PID thread.
	 * This variable is set inside the PID thread, and read outside.
	 */
	int64_t nowEncoder = 0;
	/**
	 * the is a flag to check if the timer has been allocated and the thread started.
	 */
	static bool timersAllocated;
	/**
	 * This is a list of all of the Motor objects that have been attached. As a motor is attahed,
	 *  it adds itself to this list of Motor pointers. This list is read by the PID thread and each
	 *  object in the list has loop() called. once every milisecond.
	 */
	static Motor *list[MAX_POSSIBLE_MOTORS];



	/** \brief A PID Motor class using FreeRTOS threads, ESP32Encoder and ESP32PWM
	 *
	 * This Motor class is intended to be used by RBE 1001 in the WPI Robotics Department.
	 *
	 * Motor objects can be instantiated statically. The attach method must be called before using the motor.
	 *
	 * The motor uses one timer for the ESP32PWM objects. That means the static method
	 *
	 * Motor::allocateTimer (int PWMgenerationTimer)
	 *
	 * must be called before any motor objects can be attached. This method will also start the PID thread.
	 *
	 */
	Motor(int pwmPin, int dirPin, int encAPin, int encBPin);
	virtual ~Motor();

	/**
	 * \brief Attach the motors hardware
	 *
	 * This attaches the motors to the hardware ports that were saved in the constructor
	 * @note this must only be called after timers are allocated via Motor::allocateTimers(int PWMgenerationTimer)
	 *
	 */
	void attach();
	/*
	 *  \brief effort of the motor, proportional to PWM
	 *
	 * @param effort a value from -1 to 1 representing effort
	 *        0 is brake
	 *        1 is full speed clockwise
	 *        -1 is full speed counter clockwise
	 */
	void setEffort(float effort);
	/*
	 * effort of the motor
	 * @param percent a value from -100 to 100 representing effort
	 *        0 is brake
	 *        100 is full speed clockwise
	 *        -100 is full speed counter clockwise
	 */
	void setEffortPercent(float percent)
	{
		if (!isAttached)
			attach();
		setEffort(percent * 0.01);
	}
	/*
	 * effort of the motor
	 * @return a value from -1 to 1 representing effort
	 *        0 is brake
	 *        1 is full speed clockwise
	 *        -1 is full speed counter clockwise
	 */
	float getEffort();
	/*
	 * effort of the motor
	 * @return a value from -100 to 100 representing effort
	 *        0 is brake
	 *        100 is full speed clockwise
	 *        -100 is full speed counter clockwise
	 */
	float getEffortPercent()
	{
		if (!isAttached)
			attach();
		return getEffort() * 100;
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
	void setSetpointWithTime(float newTargetInDegrees, long miliseconds,
							 interpolateMode mode);
	/**
	 * SetSpeed in degrees with time
	 * Set the setpoint for the motor in degrees
	 * This implements "Red Queen" mode running interpolation in the PID controller.

	 "Now, here, you see, it takes all the running you can do, to keep in the same place.

	 If you want to get somewhere else, you must run at least twice as fast as that!"

	 — The Red Queen, Alice In Wonderland, Lewis Carroll

	 * The way this velocity mode works is that the position target is moved forward every iteration of the PID
	 * loop. The position runs away continuously, in order to keep the velocity stable.
	 * A position increment is calculated, and added to the Position every 1ms of the loop()
	 *
	 * @param newDegreesPerSecond the new speed in degrees per second
	 */
	void setSpeed(float newDegreesPerSecond);
	/**
	 * SetSpeed in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newDegreesPerSecond the new speed in degrees per second
	 * @param miliseconds the number of miliseconds to run for
	 */
	void setSpeed(float newDegreesPerSecond, long miliseconds);
	/**
	 * MoveTo in degrees with speed
	 * Set the setpoint for the motor in degrees and the speed you want to get there
	 * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param speedDegPerSec  is the speed in degrees per second
	 */
	void moveTo(float newTargetInDegrees, float speedDegPerSec);
	/**
	 * MoveFor a relative amount in degrees with speed
	 * Set the setpoint for the motor in degrees and the speed you want to get there
	 * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
	 * @param deltaTargetInDegrees the new relative setpoint for the closed loop controller
	 * @param speedDegPerSec  is the speed in degrees per second
	 * 	 * @note this is a blocking function, it will block code for multiple seconds until the motor arrives
	 * at its given setpoint
	 */
	void moveFor(float deltaTargetInDegrees, float speedDegPerSec);

	/**
	 * \brief  wait for the motor to arrive at a setpoint
	 *
	 * @note this is a blocking function, it will block code for multiple seconds until the motor arrives
	 * at its given setpoint
	 */
	void blockUntilMoveIsDone();
	/**
	 * StartMoveFor a relative amount in degrees with speed
	 * Set the setpoint for the motor in degrees and the speed you want to get there
	 * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
	 * @param deltaTargetInDegrees the new relative setpoint for the closed loop controller
	 * @param speedDegPerSec  is the speed in degrees per second
	 */
	float startMoveFor(float deltaTargetInDegrees, float speedDegPerSec);

	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 */
	void setSetpoint(float newTargetInDegrees)
	{
		if (!isAttached)
			attach();
		setSetpointWithTime(newTargetInDegrees, 0, LINEAR_INTERPOLATION);
	}

	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param miliseconds the number of miliseconds to get from current position to the new setpoint
	 * use linear interoplation
	 */
	void setSetpointWithLinearInterpolation(float newTargetInDegrees,
											long miliseconds)
	{
		if (!isAttached)
			attach();
		setSetpointWithTime(newTargetInDegrees, miliseconds,
							LINEAR_INTERPOLATION);
	}

	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param miliseconds the number of miliseconds to get from current position to the new setpoint
	 * use sinusoidal interpolation
	 */
	void setSetpointWithSinusoidalInterpolation(float newTargetInDegrees,
												long miliseconds)
	{
		if (!isAttached)
			attach();
		setSetpointWithTime(newTargetInDegrees, miliseconds,
							SINUSOIDAL_INTERPOLATION);
	}
	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param miliseconds the number of miliseconds to get from current position to the new setpoint
	 * @param Control_0 On a scale of 0 to 1, where should the first control  point in the equation go
	 * @param Control_1 On a scale of 0 to 1, where should the second control point in the equation go
	 * use Bezier interpolation
	 */
	void setSetpointWithBezierInterpolation(float newTargetInDegrees,
											long miliseconds, float Control_0 = 0.5, float Control_1 = 1.0)
	{
		if (!isAttached)
			attach();
		BEZIER_P0 = Control_0;
		BEZIER_P1 = Control_1;
		setSetpointWithTime(newTargetInDegrees, miliseconds,
							BEZIER);
	}
	/**
	 * SetSetpoint in degrees with time
	 * Set the setpoint for the motor in degrees
	 * @param newTargetInDegrees the new setpoint for the closed loop controller
	 * @param miliseconds the number of miliseconds to get from current position to the new setpoint
	 * @param trapazoidalTime miliseconds for the ramping to take at the beginning and end.
	 *
	 *
	 * use sinusoidal interpolation
	 */
	void setSetpointWithTrapezoidalInterpolation(float newTargetInDegrees,
												 long miliseconds, float trapazoidalTime)
	{
		if (!isAttached)
			attach();
		if (trapazoidalTime * 2 > miliseconds)
		{
			setSetpointWithSinusoidalInterpolation(newTargetInDegrees,
												   miliseconds);
			return;
		}
		TRAPEZOIDAL_time = trapazoidalTime;
		setSetpointWithTime(newTargetInDegrees, miliseconds, TRAPEZOIDAL);
	}
	/**
	 * PID gains for the PID controller
	 */
	void setGains(float p, float i, float d);
	void setGainsP(float p);
	void setGainsI(float i);
	void setGainsD(float d);

	float getGainsP()
	{
		if (!isAttached)
			attach();
		return kP;
	}
	float getGainsI()
	{
		if (!isAttached)
			attach();
		return kI;
	}
	float getGainsD()
	{
		if (!isAttached)
			attach();
		return kD;
	}

	/**
	 * isMotorDoneWithMove
	 *
	 *  \brief  Check to see if the motor is done with a move
	 *
	 *  This checks that the interpolation is done,
	 *  that the position is within 1 degree
	 *  and that the velocity is close to zero
	 *
	 */
	bool isMotorDoneWithMove();
};

//extern Motor left_motor;
//extern Motor right_motor;

#endif /* LIBRARIES_RBE1001LIB_SRC_MOTOR_H_ */

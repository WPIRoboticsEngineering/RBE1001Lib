/*
 * Motor.cpp
 *
 *  Created on: May 31, 2020
 *      Author: hephaestus
 */

#include <Motor.h>
static const char *TAG = "Motor Class";

bool Motor::timersAllocated = false;
Motor *Motor::list[MAX_POSSIBLE_MOTORS] = {
	NULL,
};
static TaskHandle_t complexHandlerTask;
//portMUX_TYPE mmux = portMUX_INITIALIZER_UNLOCKED;

float myFmapBounded(float x, float in_min, float in_max, float out_min,
					float out_max)
{

	if (x > in_max)
		return out_max;
	if (x < in_min)
		return out_min;
	return ((x - in_min) * (out_max - out_min) / (in_max - in_min)) + out_min;
}
/**
 * getInterpolationUnitIncrement A unit vector from
 * 0 to 1 representing the state of the interpolation.
 */
float Motor::getInterpolationUnitIncrement()
{
	if (!isAttached)
		attach();
	float interpElapsed = (float)(millis() - startTime);
	if (interpElapsed < duration && duration > 0)
	{
		// linear elapsed duration
		unitDuration = interpElapsed / duration;
		if (mode == SINUSOIDAL_INTERPOLATION)
		{
			// sinusoidal ramp up and ramp down
			float sinPortion = (cos(-PI * unitDuration) / 2) + 0.5;
			unitDuration = 1 - sinPortion;
		}
		if (mode == BEZIER)
		{
			if (unitDuration > 0 && unitDuration < 1)
			{
				float t = unitDuration;
				float P0 = 0;
				float P1 = BEZIER_P0;
				float P2 = BEZIER_P1;
				float P3 = 1;
				unitDuration = pow((1 - t), 3) * P0 + 3 * t * pow((1 - t), 2) * P1 + 3 * pow(t, 2) * (1 - t) * P2 + pow(t, 3) * P3;
			}
		}
		if (mode == TRAPEZOIDAL)
		{
			float lengthOfLinearMode = duration - (TRAPEZOIDAL_time * 2);
			float unitLienear = lengthOfLinearMode / duration;
			float unitRamp = ((float)TRAPEZOIDAL_time) / duration;
			float unitStartRampDown = unitLienear + unitRamp;
			if (unitDuration < unitRamp)
			{
				// ramp up
				// range from 1 to 0.5
				float increment = 1 - (unitDuration) / (unitRamp * 2);
				// range 0 to 1
				float sinPortion = 1 + cos(-PI * increment);
				unitDuration = sinPortion * unitRamp;
			}
			else if (unitDuration > unitRamp && unitDuration < unitStartRampDown)
			{
				// constant speed
			}
			else if (unitDuration > unitStartRampDown)
			{
				float increment = (unitDuration - unitStartRampDown) / (unitRamp * 2) + 0.5;
				float sinPortion = 0.5 - ((cos(-PI * increment) / 2) + 0.5);
				unitDuration = (sinPortion * 2) * unitRamp + unitStartRampDown;
			}
		}
		return unitDuration;
	}
	return 1;
}
void onMotorTimer(void *param)
{
	ESP_LOGI(TAG, "Starting the PID loop thread");
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1;
	xLastWakeTime = xTaskGetTickCount();
	while (1)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		//
		for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++)
		{
			if (Motor::list[i] != NULL)
			{

				Motor::list[i]->loop();
			}
		}
		//
	}
	ESP_LOGE(TAG, "ERROR Pid thread died!");
}
/**
 * @param PWMgenerationTimer the timer to be used to generate the 20khx PWM
 * @param controllerTimer a timer running at 1khz for PID, velocity measurment, and trajectory planning
 */
void Motor::allocateTimer(int PWMgenerationTimer)
{
	if (!Motor::timersAllocated)
	{
		//ESP32PWM::allocateTimer(PWMgenerationTimer);
		xTaskCreatePinnedToCore(onMotorTimer, "PID loop Thread", 8192, NULL, 1,
								&complexHandlerTask, 0);
	}
	Motor::timersAllocated = true;
}

Motor::Motor(int pwmPin, int dirPin, int encAPin, int encBPin)
{
	MotorPWMPin = pwmPin;
	directionFlag = dirPin;
	MotorEncAPin = encAPin;
	MotorEncBPin = encBPin;
	pwm = NULL;
	encoder = NULL;
}

Motor::~Motor()
{
	encoder->pauseCount();
	pwm->detachPin(MotorPWMPin);
	delete (encoder);
	delete (pwm);
}

/**
 * setSetpoint in degrees with time
 * Set the setpoint for the motor in degrees
 * @param newTargetInDegrees the new setpoint for the closed loop controller
 * @param msTimeDuration the number of miliseconds to get from current position to the new setpoint
 */
void Motor::setSetpointWithTime(float newTargetInDegrees, long msTimeDuration,
								interpolateMode mode)
{
	if (!isAttached)
		attach();

	float newSetpoint = newTargetInDegrees / TICKS_TO_DEGREES;
	//portENTER_CRITICAL(&mmux);
	closedLoopControl = true;
	if (newSetpoint == setpoint && msTimeDuration == duration && this->mode == mode)
		return;
	startSetpoint = encoder->getCount();
	endSetpoint = newSetpoint;
	startTime = millis();
	duration = msTimeDuration;
	this->mode = mode;
	if (msTimeDuration < 1)
	{
		setpoint = newSetpoint;
	}
	ESP_LOGI(TAG, "Starting Interpolated move %.4f deg, %ld ms %d mode",
			 newTargetInDegrees, msTimeDuration, mode);
	//portEXIT_CRITICAL(&mmux);
}

/**
 * moveTo in degrees with speed
 * Set the setpoint for the motor in degrees and the speed you want to get there
 * Bascially, a wrapper function for setSetpointWithTime that takes speed as an argument
 * @param newTargetInDegrees the new setpoint for the closed loop controller
 * @param speedDegPerSec  is the speed in degrees per second
 */
void Motor::moveTo(float newTargetInDegrees, float speedDegPerSec)
{
	if (!isAttached)
		attach();

	float deltaMove = getCurrentDegrees() - newTargetInDegrees;
	setSetpointWithBezierInterpolation(newTargetInDegrees,
									   fabs(deltaMove / speedDegPerSec) * 1000.0, 0.2, 1);
}

float Motor::startMoveFor(float deltaTargetInDegrees, float speedDegPerSec)
{
	if (!isAttached)
		attach();

	float newSetPoint = getCurrentDegrees() + deltaTargetInDegrees;
	setSetpointWithBezierInterpolation(newSetPoint,
									   fabs(deltaTargetInDegrees / speedDegPerSec) * 1000.0, 0.2, 1);
	return newSetPoint;
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
bool Motor::isMotorDoneWithMove()
{
	if (!isAttached)
		attach();

	// First wait for the interpolation to finish
	if (getInterpolationUnitIncrement() < 1)
	{
		ESP_LOGD(TAG, "Move Interpolation Remaining: %.4f", getInterpolationUnitIncrement());
		return false;
	}
	float distanceToGo = fabs(
		(setpoint * TICKS_TO_DEGREES) - getCurrentDegrees());

	if (distanceToGo > 0.75)
	{ // more than 1 degree from target
		ESP_LOGD(TAG, "Move Remaining:  %.4f", distanceToGo);
		return false;
	}
	// wait for the velocity to be below 10deg/sec
	// 5deg/sec is lower bound of detection
	if (fabs(getDegreesPerSecond()) > 10)
	{
		ESP_LOGD(TAG, "Move Speed: %.4f", getDegreesPerSecond());
		return false;
	}
	// All moving checks came back passed!
	return true;
}

/**
 * \brief  wait for the motor to arrive at a setpoint
 *
 * @note this is a blocking function, it will block code for multiple seconds until the motor arrives
 * at its given setpoint
 */
void Motor::blockUntilMoveIsDone()
{
	if (!isAttached)
		attach();

	while (!isMotorDoneWithMove())
	{
		delay(10);
	}
}

/**
 * moveFor a relative amount in degrees with speed
 * Set the setpoint for the motor in degrees and the speed you want to get there
 * Bascially, a wrapper function for setSetpointWithTime that takes speed as an argument
 * @param deltaTargetInDegrees the new relative setpoint for the closed loop controller
 * @param speedDegPerSec  is the speed in degrees per second
 * @note this is a blocking function, it will block code for multiple seconds until the motor arrives
 * at its given setpoint
 */
void Motor::moveFor(float deltaTargetInDegrees, float speedDegPerSec)
{
	if (!isAttached)
		attach();

	startMoveFor(deltaTargetInDegrees, speedDegPerSec);
	blockUntilMoveIsDone();
}

/**
 * setSpeed in degrees with time
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
void Motor::setSpeed(float newDegreesPerSecond)
{
	if (!isAttached)
		attach();

	if (closedLoopControl == false)
		setSetpoint(getCurrentDegrees());
	if (fabs(newDegreesPerSecond) < 0.1)
	{
		setSetpoint(getCurrentDegrees());
		//ESP_LOGI(TAG, "Stopping");
		return;
	}
	milisecondPosIncrementForVelocity = (-newDegreesPerSecond * (((float)-1.0) / 1000.0)) / TICKS_TO_DEGREES;
	//	ESP_LOGI(TAG,"Setting Speed "+String(newDegreesPerSecond)+
	//			" increment "+String(milisecondPosIncrementForVelocity)+
	//			" scale "+String(TICKS_TO_DEGREES)
	//			+" setpoint "+String(setpoint*TICKS_TO_DEGREES)
	//	);
	//setpoint = nowEncoder;
	mode = VELOCITY_MODE;
	closedLoopControl = true;
}

/**
 * setSpeed in degrees with time
 * Set the setpoint for the motor in degrees
 * @param newDegreesPerSecond the new speed in degrees per second
 * @param miliseconds the number of miliseconds to run for
 * @note a value of 0 miliseconds will set the motor into open-ended run mode
 */
void Motor::setSpeed(float newDegreesPerSecond, long miliseconds)
{
	if (!isAttached)
		attach();

	if (miliseconds < 1)
	{
		// 0 time will set up "Red Queen" (sic) interpolation
		setSpeed(newDegreesPerSecond);
		return;
	}
	float currentPos = getCurrentDegrees();
	float distance = currentPos + (-newDegreesPerSecond * (((float)miliseconds) / 1000.0));
	setSetpointWithTime(distance, miliseconds, LINEAR_INTERPOLATION);
}

/**
 * Loop function
 * this method is called by the timer to run the PID control of the motors and ensure strict timing
 *
 */
void Motor::loop()
{
	nowEncoder = encoder->getCount();
	if (closedLoopControl)
	{
		//portEXIT_CRITICAL(&mmux);
		if (mode == VELOCITY_MODE)
		{
			if (fabs(currentEffort) < 0.95) // stall detection
				setpoint += milisecondPosIncrementForVelocity;
		}
		else
		{
			unitDuration = getInterpolationUnitIncrement();
			if (unitDuration < 1)
			{
				float setpointDiff = endSetpoint - startSetpoint;
				float newSetpoint = startSetpoint + (setpointDiff * unitDuration);
				setpoint = newSetpoint;
			}
			else
			{
				// If there is no interpoation to perform, set the setpoint to the end state
				setpoint = endSetpoint;
			}
		}
		float controlErr = setpoint - nowEncoder;

		if (getInterpolationUnitIncrement() < 1)
		{
			// no i term during interpolation
			runntingITerm = 0;
		}
		else
		{
			// shrink old values out of the sum
			runntingITerm = runntingITerm * ((I_TERM_SIZE - 1.0) / I_TERM_SIZE);
			// running sum of error
			runntingITerm += controlErr;
		}

		currentEffort =
			-(controlErr * kP + ((runntingITerm / I_TERM_SIZE) * kI));

		//portEXIT_CRITICAL(&mmux);
	}

	else
	{
		if (targetEffort > currentEffort + DELTA_EFFORT)
			currentEffort += DELTA_EFFORT;
		else if (targetEffort < currentEffort - DELTA_EFFORT)
			currentEffort -= DELTA_EFFORT;
		else
			currentEffort = targetEffort;
	}

	interruptCountForVelocity++;
	if (interruptCountForVelocity == 50)
	{
		interruptCountForVelocity = 0;
		float err = prevousCount - nowEncoder;
		cachedSpeed = err / (0.05); // ticks per second
		prevousCount = nowEncoder;
	}
	// invert the effort so that the set speed and set effort match
	setEffortLocal(currentEffort);
}
/**
 * PID gains for the PID controller
 */
void Motor::setGains(float p, float i, float d)
{
	if (!isAttached)
		attach();

	//portENTER_CRITICAL(&mmux);
	kP = p;
	kI = i;
	kD = d;
	runntingITerm = 0;
	//portEXIT_CRITICAL(&mmux);
}

void Motor::setGainsP(float p)
{
	if (!isAttached)
		attach();

	kP = p;
	runntingITerm = 0;
}
void Motor::setGainsI(float i)
{
	if (!isAttached)
		attach();

	kI = i;
	runntingITerm = 0;
}
void Motor::setGainsD(float d)
{
	if (!isAttached)
		attach();

	kD = d;
	runntingITerm = 0;
}

void Motor::attach()
{
	if (isAttached)
		return;
	isAttached = true;
	// Motor timer must be allocated and the thread must be started before starting
	if (!Motor::timersAllocated)
	{
		Motor::allocateTimer(0); // used by the DC Motors
	}

	pwm = new ESP32PWM();
	encoder = new ESP32Encoder();
	ESP32Encoder::useInternalWeakPullResistors = UP;

	pwm->attachPin(MotorPWMPin, 20000, 12);
	encoder->attachFullQuad(MotorEncAPin, MotorEncBPin);
	pinMode(directionFlag, OUTPUT);
	// add the motor to the list of timer based controls
	for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++)
	{
		if (Motor::list[i] == NULL)
		{
			//			String message ="Allocating Motor " + String(i) + " on PWM "+ String(MotorPWMPin);
			//			ESP_LOGI(TAG,message.c_str());
			Motor::list[i] = this;
			return;
		}
	}
}

/*
 *  \brief effort of the motor, proportional to PWM
 *
 * @param effort a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
void Motor::setEffort(float effort)
{
	if (!isAttached)
		attach();

	if (effort > 1)
		effort = 1;
	if (effort < -1)
		effort = -1;
	//portENTER_CRITICAL(&mmux);
	closedLoopControl = false;
	targetEffort = effort;
	//portEXIT_CRITICAL(&mmux);
}
/*
 * effort of the motor
 * @return a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
float Motor::getEffort()
{
	if (!isAttached)
		attach();
	return currentEffort;
}
/*
 * effort of the motor
 * @param effort a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
void Motor::setEffortLocal(float effort)
{
	if (!isAttached)
		attach();

	if (effort > 1)
		effort = 1;
	if (effort < -1)
		effort = -1;
	if (effort > 0)
		digitalWrite(directionFlag, LOW);
	else
		digitalWrite(directionFlag, HIGH);
	pwm->writeScaled(fabs(effort));
}
/**
 * getDegreesPerSecond
 *
 * This function returns the current speed of the motor
 *
 * @return the speed of the motor in degrees per second
 */
float Motor::getDegreesPerSecond()
{
	if (!isAttached)
		attach();

	float tmp;
	//portENTER_CRITICAL(&mmux);
	tmp = cachedSpeed;
	//portEXIT_CRITICAL(&mmux);
	return -tmp * TICKS_TO_DEGREES;
}
/**
 * getTicks
 *
 * This function returns the current count of encoders
 * @return count
 */
float Motor::getCurrentDegrees()
{
	if (!isAttached)
		attach();

	float tmp;
	//portENTER_CRITICAL(&mmux);
	tmp = nowEncoder;
	//portEXIT_CRITICAL(&mmux);
	return tmp * TICKS_TO_DEGREES;
}

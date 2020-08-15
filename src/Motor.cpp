/*
 * Motor.cpp
 *
 *  Created on: May 31, 2020
 *      Author: hephaestus
 */

#include <Motor.h>

bool Motor::timersAllocated = false;
Motor * Motor::list[MAX_POSSIBLE_MOTORS] = { NULL, };
static TaskHandle_t complexHandlerTask;


float TICKS_PER_DEGREE = 1.0/TICKS_TO_DEGREES;

const uint32_t LOOP_PERIOD_MS = 10;

float myFmapBounded(float x, float in_min, float in_max, float out_min,
		float out_max) {
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
// float Motor::getInterpolationUnitIncrement() {
// 	float interpElapsed = (float) (millis() - startTime);
// 	if (interpElapsed < duration && duration > 0) {
// 		// linear elapsed duration
// 		unitDuration = interpElapsed / duration;
// 		if (mode == SINUSOIDAL_INTERPOLATION) {
// 			// sinusoidal ramp up and ramp down
// 			float sinPortion = (cos(-PI * unitDuration) / 2) + 0.5;
// 			unitDuration = 1 - sinPortion;
// 		}
// 		if(mode==BEZIER){
// 			if(unitDuration>0 &&unitDuration<1){
// 				float t=unitDuration;
// 				float P0=0;
// 				float P1=BEZIER_P0;
// 				float P2=BEZIER_P1;
// 				float P3=1;
// 				unitDuration= 	pow((1-t),3) *P0 +
// 								3*t*pow((1-t),2)*P1 +
// 								3*pow(t,2)*(1-t)*P2 +
// 								pow(t,3)*P3;
// 			}
// 		}
// 		if(mode == TRAPEZOIDAL){
// 			float lengthOfLinearMode = duration-(TRAPEZOIDAL_time*2);
// 			float unitLienear = lengthOfLinearMode/duration;
// 			float unitRamp = ((float)TRAPEZOIDAL_time)/duration;
// 			float unitStartRampDown = unitLienear+unitRamp;
// 			if(unitDuration<unitRamp){
// 				// ramp up
// 				// range from 1 to 0.5
// 				float increment =1- (unitDuration)/(unitRamp*2);
// 				// range 0 to 1
// 				float sinPortion = 1+cos(-PI *increment);
// 				unitDuration=sinPortion*unitRamp;
// 			}
// 			else if(unitDuration>unitRamp&&unitDuration<unitStartRampDown){
// 				// constant speed

// 			}
// 			else if(unitDuration>unitStartRampDown){
// 				float increment=(unitDuration-unitStartRampDown)/(unitRamp*2)+0.5;
// 				float sinPortion = 0.5-((cos(-PI *increment) / 2) + 0.5);
// 				unitDuration = (sinPortion*2)*unitRamp+unitStartRampDown;
// 			}
// 		}
// 		return unitDuration;
// 	}
// 	return 1;
// }

void onMotorTimer(void *param) {
	Serial.println("Starting the PID loop thread");
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = LOOP_PERIOD_MS;
	xLastWakeTime = xTaskGetTickCount();
	while (1) {
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		//
		for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++) {
			if (Motor::list[i] != NULL) {

				Motor::list[i]->loop();

			}
		}
		//
	}
	Serial.println("ERROR Pid thread died!");

}
/**
 * @param PWMgenerationTimer the timer to be used to generate the 20khx PWM
 * @param controllerTimer a timer running at 1khz for PID, velocity measurment, and trajectory planning
 */
void Motor::allocateTimer(int PWMgenerationTimer) {
	if (!Motor::timersAllocated) {
		ESP32PWM::allocateTimer(PWMgenerationTimer);
		xTaskCreatePinnedToCore(onMotorTimer, "PID loop Thread", 8192, NULL, 1,
				&complexHandlerTask, 0);
	}
	Motor::timersAllocated = true;
}

Motor::Motor() {
	pwm = NULL;
	encoder = NULL;
}

Motor::~Motor() {
	encoder->pauseCount();
	pwm->detachPin(MotorPWMPin);
	delete (encoder);
	delete (pwm);
}

// /**
//  * SetSetpoint in degrees with time
//  * Set the setpoint for the motor in degrees
//  * @param newTargetInDegrees the new setpoint for the closed loop controller
//  * @param msTimeDuration the number of miliseconds to get from current position to the new setpoint
//  */
// void Motor::SetSetpointWithTime(float newTargetInDegrees, long msTimeDuration,
// 		interpolateMode mode) {
// 	float newSetpoint = newTargetInDegrees / TICKS_TO_DEGREES;
// 	//portENTER_CRITICAL(&mmux);
// 	closedLoopControl = true;
// 	if (newSetpoint == Setpoint && msTimeDuration == duration
// 			&& this->mode == mode)
// 		return;
// 	startSetpoint = encoder->getCount();
// 	endSetpoint = newSetpoint;
// 	startTime = millis();
// 	duration = msTimeDuration;
// 	this->mode = mode;
// 	if (msTimeDuration < 1) {
// 		Setpoint = newSetpoint;
// 	}
// 	//portEXIT_CRITICAL(&mmux);
// }

/**
 * MoveTo in degrees with speed
 * Set the setpoint for the motor in degrees and the speed you want to get there
 * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
 * @param newTargetInDegrees the new setpoint for the closed loop controller
 * @param speedDegPerSec  is the speed in degrees per second
*/
void Motor::MoveTo(float newTargetInDegrees, float speedDegPerSec)
{
	StartMoveTo(newTargetInDegrees, speedDegPerSec);
	while(currTrajectory.FractionComplete() < 1.0) {Serial.println(currTrajectory.FractionComplete());}

	return;
}


float Motor::StartMoveTo(float newTargetInDegrees, float speedDegPerSec)
{
	currTrajectory.startPos = getCurrentDegrees() * TICKS_PER_DEGREE;
	currTrajectory.targetPos = newTargetInDegrees * TICKS_PER_DEGREE;

	if(currTrajectory.targetPos > currTrajectory.startPos) speedDegPerSec = fabs(speedDegPerSec);
	else speedDegPerSec = -fabs(speedDegPerSec);

	SetDelta(speedDegPerSec);
	mode = LINEAR_INTERPOLATION; 

	return newTargetInDegrees;
}

float Motor::StartMoveFor(float deltaTargetInDegrees, float speedDegPerSec)
{
	float targetPosDeg = getCurrentDegrees() + deltaTargetInDegrees;
	return StartMoveTo(targetPosDeg, speedDegPerSec);
}

// /**
//  * \brief  wait for the motor to arrive at a setpoint
//  *
//  * @note this is a blocking function, it will block code for multiple seconds until the motor arrives
//  * at its given setpoint
//  */
// void Motor::blockUntilMoveIsDone()
// {
// 	float distanceToGo;
// 	// First wait for the interpolation to finish
// 	while(getInterpolationUnitIncrement()<1){
// 		delay(10);
// 		Serial.println(" Interpolation "+String (getInterpolationUnitIncrement()));
// 	}
// 	do
// 	{
// 		delay(10);
// 		distanceToGo=fabs((Setpoint*TICKS_TO_DEGREES) - getCurrentDegrees());
// 		Serial.println("Remaining: "+String(distanceToGo));
// 	}while (distanceToGo>TICKS_TO_DEGREES );// get within 1 tick
// 	// wait for the velocity to be below 10deg/sec
// 	// 5deg/sec is lower bound of detection
// 	while (fabs(getDegreesPerSecond()) > 10) {
// 		Serial.println("Speed: "+String(getDegreesPerSecond()));
// 		delay(10);
// 	}
// }
/**
 * MoveFor a relative amount in degrees with speed
 * Set the setpoint for the motor in degrees and the speed you want to get there
 * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
 * @param deltaTargetInDegrees the new relative setpoint for the closed loop controller
 * @param speedDegPerSec  is the speed in degrees per second
 * @note this is a blocking function, it will block code for multiple seconds until the motor arrives
 * at its given setpoint
 */
void Motor::MoveFor(float deltaTargetInDegrees, float speedDegPerSec)
{
	StartMoveFor(deltaTargetInDegrees, speedDegPerSec);
	while(currTrajectory.FractionComplete() < 1.0) {Serial.println(currTrajectory.FractionComplete());}
}

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
void Motor::SetSpeed(float speedDegPerSec) 
{
	SetDelta(speedDegPerSec);
	mode = VELOCITY_MODE;
}

float Motor::SetDelta(float speedDegPerSec)
{
	deltaTargetTicks = speedDegPerSec * TICKS_PER_DEGREE * LOOP_PERIOD_MS / 1000.0;
	Serial.println("Setting Speed "+String(speedDegPerSec)+
			" increment "+String(deltaTargetTicks)
	);

	Setpoint = nowEncoder;
	closedLoopControl = true;

	return deltaTargetTicks;
}

/**
 * SetSpeed in degrees with time
 * Set the setpoint for the motor in degrees
 * @param newDegreesPerSecond the new speed in degrees per second
 * @param miliseconds the number of miliseconds to run for
 * @note a value of 0 miliseconds will set the motor into open-ended run mode
 */
void Motor::SetSpeed(float newDegreesPerSecond, long miliseconds) {
	if (miliseconds < 1) {
		// 0 time will set up "Red Queen" (sic) interpolation
		SetSpeed(newDegreesPerSecond);
		return;
	}
	float currentPos = getCurrentDegrees();
	float distance = currentPos
			+ (-newDegreesPerSecond * (((float) miliseconds) / 1000.0));
	SetSetpointWithTime(distance, miliseconds, LINEAR_INTERPOLATION);
}

/**
 * Loop function
 * this method is called by the timer to run the PID control of the motors and ensure strict timing
 *
 */
void Motor::loop() 
{
	nowEncoder = encoder->getCount();
	currTrajectory.currPos = nowEncoder;

	if (closedLoopControl) 
	{
		if (mode == VELOCITY_MODE) 
		{
			if(fabs(currentEffort)<0.95)// stall detection -- could be less, could be better
				Setpoint += deltaTargetTicks;
		} 
		
		else 
		{
			if(currTrajectory.FractionComplete() >= 1.0)
			{
				Setpoint = currTrajectory.targetPos;
			}	
			else if(fabs(currentEffort)<0.5)// stall detection -- could be less, could be better
				Setpoint += deltaTargetTicks;
		}

		float controlErr = Setpoint - nowEncoder;

		if(fabs(currentEffort)<0.5) runningITerm += controlErr;

		currentEffort = controlErr * kP + runningITerm * kI;
	}

	//this needs redoing
	interruptCountForVelocity++;
	if (interruptCountForVelocity == 50) {
		interruptCountForVelocity = 0;
		float err = prevousCount - nowEncoder;
		cachedSpeed = err / (0.05); // ticks per second
		prevousCount = nowEncoder;
	}
	// invert the effort so that the set speed and set effort match
	SetEffortLocal(currentEffort);
}
/**
 * PID gains for the PID controller
 */
void Motor::setGains(float p, float i, float d) {
	//portENTER_CRITICAL(&mmux);
	kP = p;
	kI = i;
	kD = d;
	runningITerm = 0;
	//portEXIT_CRITICAL(&mmux);
}


void Motor::attach(int MotorPWMPin, int MotorDirectionPin, int EncoderA,
		int EncoderB) {
	// Motor timer must be allocated and the thread must be started before starting
	if (!Motor::timersAllocated){
		Motor::allocateTimer(0); // used by the DC Motors
	}
	pwm = new ESP32PWM();
	encoder = new ESP32Encoder();
	ESP32Encoder::useInternalWeakPullResistors = UP;

	pwm->attachPin(MotorPWMPin, 20000, 8);
	directionFlag = MotorDirectionPin;
	this->MotorPWMPin = MotorPWMPin;
	encoder->attachFullQuad(EncoderA, EncoderB);
	pinMode(directionFlag, OUTPUT);
	// add the motor to the list of timer based controls
	for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++) {
		if (Motor::list[i] == NULL) {

			Serial.println(
					"Allocating Motor " + String(i) + " on PWM "
							+ String(MotorPWMPin));
			Motor::list[i] = this;
			return;
		}

	}
}

/*
 *  \brief effort of the motor, proportional to PWM
 *
 * @param a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
void Motor::SetEffort(float effort) { closedLoopControl = false; SetEffortLocal(effort); }
/*
 * effort of the motor
 * @return a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
float Motor::GetEffort() {
	return currentEffort;
}
/*
 * effort of the motor
 * @param a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
void Motor::SetEffortLocal(float effort) {
	if (effort > 1)
		effort = 1;
	if (effort < -1)
		effort = -1;
	if (effort > 0)
		digitalWrite(directionFlag, HIGH);
	else
		digitalWrite(directionFlag, LOW);
	currentEffort = effort;
	pwm->writeScaled(abs(effort));
}
/**
 * getDegreesPerSecond
 *
 * This function returns the current speed of the motor
 *
 * @return the speed of the motor in degrees per second
 */
float Motor::getDegreesPerSecond() {
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
float Motor::getCurrentDegrees() {
	float tmp;
	//portENTER_CRITICAL(&mmux);
	tmp = nowEncoder;
	//portEXIT_CRITICAL(&mmux);
	return tmp * TICKS_TO_DEGREES;
}


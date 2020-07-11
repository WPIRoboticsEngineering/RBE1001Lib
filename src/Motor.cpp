/*
 * Motor.cpp
 *
 *  Created on: May 31, 2020
 *      Author: hephaestus
 */

#include <Motor.h>
hw_timer_t *Motor::timer = NULL;

bool Motor::timersAllocated = false;
Motor * Motor::list[MAX_POSSIBLE_MOTORS] = { NULL, };
static TaskHandle_t complexHandlerTask;
//portMUX_TYPE mmux = portMUX_INITIALIZER_UNLOCKED;

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
float Motor::getInterpolationUnitIncrement(){
	float interpElapsed = (float) (millis() - startTime);
	if(interpElapsed < duration && duration > 0){
		// linear elapsed duration
		unitDuration = interpElapsed / duration;
		if (mode == SINUSOIDAL_INTERPOLATION) {
			// sinusoidal ramp up and ramp down
			float sinPortion = (cos(-PI * unitDuration) / 2) + 0.5;
			unitDuration = 1 - sinPortion;
		}
		return unitDuration;
	}
	return 1;
}
void onMotorTimer(void *param) {
	Serial.println("Starting the PID loop thread");
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1;
	xLastWakeTime = xTaskGetTickCount();
	while (1) {
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
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
				&complexHandlerTask,0);
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

/**
 * SetSetpoint in degrees with time
 * Set the setpoint for the motor in degrees
 * @param newTargetInDegrees the new setpoint for the closed loop controller
 * @param msTimeDuration the number of miliseconds to get from current position to the new setpoint
 */
void Motor::SetSetpointWithTime(float newTargetInDegrees, long msTimeDuration,
		interpolateMode mode){
	float newSetpoint =newTargetInDegrees/TICKS_TO_DEGREES;
	//portENTER_CRITICAL(&mmux);
	closedLoopControl=true;
	if(newSetpoint==Setpoint &&msTimeDuration== duration&& this->mode==mode)
		return;
	startSetpoint = Setpoint;
	endSetpoint = newSetpoint;
	startTime = millis();
	duration = msTimeDuration;
	this->mode = mode;
	if(msTimeDuration<1){
		Setpoint=newSetpoint;
	}
	//portEXIT_CRITICAL(&mmux);
}

/**
 * MoveTo in degrees with speed
 * Set the setpoint for the motor in degrees and the speed you want to get there
 * Bascially, a wrapper function for SetSetpointWithTime that takes speed as an argument
 * @param newTargetInDegrees the new setpoint for the closed loop controller
 * @param speedDegPerSec  is the speed in degrees per second
*/
void Motor::MoveTo(float newTargetInDegrees, float speedDegPerSec)
{
    SetSetpointWithTime(newTargetInDegrees, (newTargetInDegrees/speedDegPerSec) * 1000.0, LINEAR_INTERPOLATION);
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
void Motor::SetSpeed(float newDegreesPerSecond){
	if(abs(newDegreesPerSecond)<0.1){
		SetSetpoint(getCurrentDegrees());
//		Serial.println("Stopping");
		return;
	}
	milisecondPosIncrementForVelocity=(newDegreesPerSecond * (((float) -1.0) / 1000.0))/TICKS_TO_DEGREES;
//	Serial.println("Setting Speed "+String(newDegreesPerSecond)+
//			" increment "+String(milisecondPosIncrementForVelocity)+
//			" scale "+String(TICKS_TO_DEGREES)
//			+" Setpoint "+String(Setpoint*TICKS_TO_DEGREES)
//	);
	Setpoint=nowEncoder;
	mode=VELOCITY_MODE;
	closedLoopControl=true;
}

/**
 * SetSpeed in degrees with time
 * Set the setpoint for the motor in degrees
 * @param newDegreesPerSecond the new speed in degrees per second
 * @param miliseconds the number of miliseconds to run for
 * @note a value of 0 miliseconds will set the motor into open-ended run mode
 */
void Motor::SetSpeed(float newDegreesPerSecond, long miliseconds) {
	if(miliseconds<1){
		// 0 time will set up "Red Queen" (sic) interpolation
		SetSpeed(newDegreesPerSecond);
		return;
	}
	float currentPos = getCurrentDegrees();
	float distance = currentPos
			+ (newDegreesPerSecond * (((float) miliseconds) / 1000.0));
	SetSetpointWithTime(distance, miliseconds, LINEAR_INTERPOLATION);
}

/**
 * Loop function
 * this method is called by the timer to run the PID control of the motors and ensure strict timing
 *
 */
void Motor::loop() {
	nowEncoder = encoder->getCount();
	if(closedLoopControl){
		//portEXIT_CRITICAL(&mmux);
		if(mode ==VELOCITY_MODE){
			Setpoint+=milisecondPosIncrementForVelocity;
		}else{
			unitDuration=getInterpolationUnitIncrement();
			if (unitDuration<1) {
				float setpointDiff = endSetpoint - startSetpoint;
				float newSetpoint = startSetpoint + (setpointDiff * unitDuration);
				Setpoint = newSetpoint;
			} else {
				// If there is no interpoation to perform, set the setpoint to the end state
				Setpoint = endSetpoint;
			}
		}
		float controlErr = Setpoint-nowEncoder;
		// shrink old values out of the sum
		runntingITerm=runntingITerm*((I_TERM_SIZE-1.0)/I_TERM_SIZE);
		// running sum of error
		runntingITerm+=controlErr;

		currentEffort=controlErr*kP+((runntingITerm/I_TERM_SIZE)*kI);

		//portEXIT_CRITICAL(&mmux);
	}

	interruptCountForVelocity++;
	if (interruptCountForVelocity == 50) {
		interruptCountForVelocity = 0;
		float err = prevousCount - nowEncoder;
		cachedSpeed = err / (0.05); // ticks per second
		prevousCount = nowEncoder;
	}
	SetEffortLocal(currentEffort);

}
/**
 * PID gains for the PID controller
 */
void Motor::setGains(float p,float i,float d){
	//portENTER_CRITICAL(&mmux);
	kP=p;
	kI=i;
	kD=d;
	runntingITerm=0;
	//portEXIT_CRITICAL(&mmux);
}

/**
 * Attach the motors hardware
 * @param MotorPWMPin the pin that produce PWM at 20kHz (Max is 250khz per DRV8838 datasheet)
 * @param MotorDirectionPin motor direction setting pin
 * @param the A channel of the encoder
 * @param the B channel of the encoder
 * @note this must only be called after timers are allocated via Motor::allocateTimers
 *
 */
void Motor::attach(int MotorPWMPin, int MotorDirectionPin, int EncoderA,
		int EncoderB) {
	pwm = new ESP32PWM();
	encoder = new ESP32Encoder();
	ESP32Encoder::useInternalWeakPullResistors = UP;

	pwm->attachPin(MotorPWMPin, 20000, 8);
	directionFlag = MotorDirectionPin;
	this->MotorPWMPin=MotorPWMPin;
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
 * effort of the motor
 * @param a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
void Motor::SetEffort(float effort) {
	if (effort>1)
		effort=1;
	if(effort<-1)
		effort=-1;
	//portENTER_CRITICAL(&mmux);
	closedLoopControl=false;
	currentEffort=effort;
	//portEXIT_CRITICAL(&mmux);
}
/*
 * effort of the motor
 * @return a value from -1 to 1 representing effort
 *        0 is brake
 *        1 is full speed clockwise
 *        -1 is full speed counter clockwise
 */
float  Motor::GetEffort(){
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
	if (effort>1)
		effort=1;
	if(effort<-1)
		effort=-1;
	if(effort>0)
		digitalWrite(directionFlag, HIGH);
	else
		digitalWrite(directionFlag, LOW);
	currentEffort=effort;
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
	return tmp*TICKS_TO_DEGREES;
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
	return tmp*TICKS_TO_DEGREES;
}


/*
 * Motor.cpp
 *
 *  Created on: May 31, 2020
 *      Author: hephaestus
 */

#include <Motor.h>
hw_timer_t *Motor::timer = NULL;

bool Motor::timersAllocated = false;
Motor * Motor::list[MAX_POSSIBLE_MOTORS] = { NULL,
NULL, NULL, NULL };
static TaskHandle_t complexHandlerTask;

void onMotorTimer(void *param) {
	Serial.println("Starting the PID loop thread");
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1;
	xLastWakeTime = xTaskGetTickCount();
	while (1) {
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++) {
			if (Motor::list[i] != NULL) {
				Motor::list[i]->loop();
			}
		}
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

/**
 * Loop function
 * this method is called by the timer to run the PID control of the motors and ensure strict timing
 *
 */
void Motor::loop() {
	nowEncoder = encoder->getCount();
	float controlErr = Setpoint-nowEncoder;

	SetEffort(controlErr*kP);
	interruptCountForVelocity++;
	if (interruptCountForVelocity == 50) {
		interruptCountForVelocity = 0;
		float err = prevousCount - nowEncoder;
		cachedSpeed = err / (0.05); // ticks per second
		prevousCount = nowEncoder;
	}
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
	encoder->attachHalfQuad(EncoderA, EncoderB);
	pinMode(directionFlag, OUTPUT);
	// add the motor to the list of timer based controls
	for (int i = 0; i < MAX_POSSIBLE_MOTORS; i++) {
		if (Motor::list[i] == NULL) {
			Motor::list[i] = this;
			Serial.println(
					"Allocating Motor " + String(i) + " on PWM "
							+ String(MotorPWMPin));
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
	if(effort>0)
		digitalWrite(directionFlag, HIGH);
	else
		digitalWrite(directionFlag, LOW);
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
	tmp = cachedSpeed;
	return tmp;
}
/**
 * getTicks
 *
 * This function returns the current count of encoders
 * @return count
 */
int Motor::getCurrentTicks() {
	return nowEncoder;
}


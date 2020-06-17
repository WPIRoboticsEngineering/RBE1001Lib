#include <Arduino.h>
#include <RBE1001Lib.h>
#include "Motor.h"
#include "Rangefinder.h"
#include <ESP32Servo.h>
#include <ESP32AnalogRead.h>

Motor motor1;
Motor motor2;
Rangefinder rangefinder1;
Servo lifter;
ESP32AnalogRead leftLineSensor;
ESP32AnalogRead rightLineSensor;
ESP32AnalogRead servoPositionFeedback;
bool upDown=false;
/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 */
void setup() {
  // This will initialize the Serial as 115200 for prints
  Serial.begin(115200);
  Motor::allocateTimer(0); // used by the DC Motors
  ESP32PWM::allocateTimer(1);// Used by servos
  motor2.attach(MOTOR2_PWM, MOTOR2_DIR, MOTOR2_ENCA, MOTOR2_ENCB);
  motor1.attach(MOTOR1_PWM, MOTOR1_DIR, MOTOR1_ENCA, MOTOR1_ENCB);
  rangefinder1.attach(SIDE_ULTRASONIC_TRIG, SIDE_ULTRASONIC_ECHO);
  lifter.attach(SERVO_PIN, 1000, 2000);
  leftLineSensor.attach(LEFT_LINE_SENSE);
  rightLineSensor.attach(RIGHT_LINE_SENSE);
  servoPositionFeedback.attach(SERVO_FEEDBACK_SENSOR);

}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() {
	upDown=!upDown;
	int loopTime = 4000;// 4 second loop
	int servoRange =180;
	motor2.SetSetpointWithSinusoidalInterpolation(upDown?360:0, loopTime);
	motor1.SetSetpointWithSinusoidalInterpolation(upDown?360:0, loopTime);

	for(int i=0;i<servoRange;i++){
		delay(loopTime/servoRange);
		Serial.println("\n");
		delay(1);
		Serial.println("Range 1 "+String(rangefinder1.getDistanceCM())+" T-O: "+String(Rangefinder::getTimeoutState()));delay(1);
		Serial.println("Speed 1 "+String(motor1.getDegreesPerSecond())+
					" Speed 2 "+String(motor2.getDegreesPerSecond()));delay(1);
		Serial.println("Count 1 "+String(motor1.getCurrentDegrees())+
						" Count 2 "+String(motor2.getCurrentDegrees()));delay(1);
		Serial.println("Line Sense left "+String(leftLineSensor.readVoltage()));delay(1);

		Serial.println(	"Line Sense right "+String(rightLineSensor.readVoltage()));delay(1);
		Serial.println("Servo Read Position "+String(servoPositionFeedback.readVoltage()));delay(1);
		Serial.println("Servo write Position "+String(i));delay(1);
		lifter.write(i);delay(1);
	}




 }


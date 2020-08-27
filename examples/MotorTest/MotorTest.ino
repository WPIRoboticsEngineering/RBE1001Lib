#include <Arduino.h>
#include <RBE1001Lib.h>

Motor left_motor;
Motor right_motor;
bool upDown=false;
/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 */
void setup() {
  // This will initialize the Serial as 115200 for prints
  Serial.begin(115200);
  // Set up the motors Timer for use in making PWM's
  Motor::allocateTimer(0);
  // pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
  left_motor.attach(MOTOR_LEFT_PWM, MOTOR_LEFT_DIR, MOTOR_LEFT_ENCA, MOTOR_LEFT_ENCB);
  right_motor.attach(MOTOR_RIGHT_PWM, MOTOR_RIGHT_DIR, MOTOR_RIGHT_ENCA, MOTOR_RIGHT_ENCB);
}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() {
	upDown=!upDown;
	left_motor.moveTo(upDown?360*2.5:0, 360);
	right_motor.moveTo(upDown?360*2.5:0, 360);

	// Check both motors to see if they are don moving yet
	while(!left_motor.isMotorDoneWithMove() &&
			!right_motor.isMotorDoneWithMove())
	{
		// print the state of the motors while running
		delay(20);
		Serial.println("motor compared  "+String(right_motor.getInterpolationUnitIncrement()-left_motor.getInterpolationUnitIncrement())+
				+" Interp "+String(right_motor.getInterpolationUnitIncrement())+
				+" Vel 1 "+String(left_motor.getDegreesPerSecond())+" Vel 2 "+String(right_motor.getDegreesPerSecond()));
	}
	// Print the count at the end of a loop
	delay(100);
	Serial.println("Count 1 "+String(left_motor.getCurrentDegrees())+
					" Count 2 "+String(right_motor.getCurrentDegrees()));
	delay(1000);


 }


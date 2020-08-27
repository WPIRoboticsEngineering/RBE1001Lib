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
	left_motor.moveTo(upDown?1000:0, 360);
	//left_motor.SetSetpointWithLinearInterpolation(upDown?3600:0, 8000);
	//right_motor.SetSetpointWithLinearInterpolation(upDown?360:0, 2000);
	//right_motor.SetSetpointWithBezierInterpolation(upDown?3600:0, 8000,0.45,1);
	//right_motor.setSetpointWithTrapezoidalInterpolation(upDown?3600:0, 8000, 500);
	double peak1 = 0;
	double peak2 =0;

	while(!left_motor.isMotorDoneWithMove()){
		if(abs(left_motor.getDegreesPerSecond())>peak1){
			peak1=abs(left_motor.getDegreesPerSecond());
		}
		if(abs(right_motor.getDegreesPerSecond())>peak2){
			peak2=abs(right_motor.getDegreesPerSecond());
		}
		delay(20);
		Serial.println("motor compared  "+String(right_motor.getInterpolationUnitIncrement()-left_motor.getInterpolationUnitIncrement())+
				+" Interp "+String(right_motor.getInterpolationUnitIncrement())+
				+" Vel 1 "+String(left_motor.getDegreesPerSecond())+" Vel 2 "+String(right_motor.getDegreesPerSecond()));
	}
	delay(100);
	Serial.println("Count 1 "+String(left_motor.getCurrentDegrees())+
					" Count 2 "+String(right_motor.getCurrentDegrees()));
	delay(1000);


 }


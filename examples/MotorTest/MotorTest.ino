#include <Arduino.h>
#include <RBE1001Lib.h>

Motor motor1;
Motor motor2;
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
  motor1.attach(MOTOR1_PWM, MOTOR1_DIR, MOTOR1_ENCA, MOTOR1_ENCB);
  motor2.attach(MOTOR2_PWM, MOTOR2_DIR, MOTOR2_ENCA, MOTOR2_ENCB);
}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() {
	upDown=!upDown;
	motor1.SetSetpointWithSinusoidalInterpolation(upDown?3600:0, 8000);
	//motor2.SetSetpointWithLinearInterpolation(upDown?360:0, 2000);
	motor2.SetSetpointWithBezierInterpolation(upDown?3600:0, 8000,0.15,1.0);
	for(int i=0;i<400;i++){
		delay(20);
		Serial.println("motor interpolation  "+String(motor2.getInterpolationUnitIncrement()));
	}
	delay(1000);
	Serial.println("Count 1 "+String(motor1.getCurrentDegrees())+
				" Count 2 "+String(motor2.getCurrentDegrees()));

 }


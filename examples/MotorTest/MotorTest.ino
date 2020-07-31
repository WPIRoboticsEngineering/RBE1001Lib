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
	motor1.SetSetpointWithLinearInterpolation(upDown?3600:0, 8000);
	//motor2.SetSetpointWithLinearInterpolation(upDown?360:0, 2000);
	//motor2.SetSetpointWithBezierInterpolation(upDown?3600:0, 8000,0.45,1);
	motor2.SetSetpointWithTrapezoidalInterpolation(upDown?3600:0, 8000, 500);
	double peak1 = 0;
	double peak2 =0;

	for(int i=0;i<400;i++){
		if(abs(motor1.getDegreesPerSecond())>peak1){
			peak1=abs(motor1.getDegreesPerSecond());
		}
		if(abs(motor2.getDegreesPerSecond())>peak2){
			peak2=abs(motor2.getDegreesPerSecond());
		}
		delay(20);
		Serial.println("motor compared  "+String(motor2.getInterpolationUnitIncrement()-motor1.getInterpolationUnitIncrement())+
				+" Interp "+String(motor2.getInterpolationUnitIncrement())+
				+" Vel 1 "+String(motor1.getDegreesPerSecond())+" Vel 2 "+String(motor2.getDegreesPerSecond()));
	}
	delay(100);
	Serial.println("Count 1 "+String(motor1.getCurrentDegrees())+
					" Count 2 "+String(motor2.getCurrentDegrees()));
	delay(5000);


 }


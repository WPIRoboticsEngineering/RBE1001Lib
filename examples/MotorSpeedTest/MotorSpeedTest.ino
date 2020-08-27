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
void loop() 
{
	float speed = 360 * sin(2*3.14*(millis()%10000)/10000.);
	left_motor.setSpeed(speed);
	right_motor.setSpeed(speed);

	Serial.print("SP: ");
	Serial.print(speed);
	Serial.print('\t');

	Serial.print("L: ");
	Serial.print(left_motor.getDegreesPerSecond());
	Serial.print('\t');

	Serial.print("R: ");
	Serial.print(right_motor.getDegreesPerSecond());
	Serial.print('\n');

	delay(100);
}

// {
// 	upDown=!upDown;
// 	// Set a speed for a specific amount of time, in this case 2 seconds
// 	right_motor.SetSpeed(upDown?180:-90, upDown?2000:4000);
// 	float start = left_motor.getCurrentDegrees();
// 	long startTime= millis();
// 	for(int i=0;i<100;i++){
// 		delay(upDown?20:40);
// 		Serial.println("Speed 1="+String(left_motor.getDegreesPerSecond())+
// 					"deg/sec, Effort= "+String(left_motor.GetEffortPercent()));
// 	}
// 	Serial.println("\n");
// 	// stop the motor
// 	//left_motor.SetSpeed(0);

// 	float elapsed =((float) (millis()-startTime))/1000.0;
// 	float distanceElapsed = left_motor.getCurrentDegrees()-		start;
// 	float durationSpeed= distanceElapsed/(elapsed);
// 	Serial.println("Computed speed "+String(durationSpeed)+
// 					" measured speed "+String(left_motor.getDegreesPerSecond()));
// 	Serial.println("Time elapsed "+String(elapsed)+
// 				" Distance Elapsed  "+String(distanceElapsed)+
// 				" Started at  "+String(start)+" arrived at "+String( left_motor.getCurrentDegrees()));

// 	delay(5000);
// 	// start the motor spinning again
// 	//left_motor.SetSpeed(90);

//  }


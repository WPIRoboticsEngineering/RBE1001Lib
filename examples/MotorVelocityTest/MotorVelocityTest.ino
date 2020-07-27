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
  // Use velocity control mode with open ended time
  motor1.SetSpeed(-360);
}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() {
	upDown=!upDown;
	// Set a speed for a specific amount of time, in this case 2 seconds
	motor2.SetSpeed(upDown?180:-90, upDown?2000:4000);
	float start = motor1.getCurrentDegrees();
	long startTime= millis();
	for(int i=0;i<100;i++){
		delay(upDown?20:40);
		Serial.println("Speed 1="+String(motor1.getDegreesPerSecond())+
					"deg/sec, Effort= "+String(motor1.GetEffortPercent()));
	}
	Serial.println("\n");
	// stop the motor
	//motor1.SetSpeed(0);

	float elapsed =((float) (millis()-startTime))/1000.0;
	float distanceElapsed = motor1.getCurrentDegrees()-		start;
	float durationSpeed= distanceElapsed/(elapsed);
	Serial.println("Computed speed "+String(durationSpeed)+
					" measured speed "+String(motor1.getDegreesPerSecond()));
	Serial.println("Time elapsed "+String(elapsed)+
				" Distance Elapsed  "+String(distanceElapsed)+
				" Started at  "+String(start)+" arrived at "+String( motor1.getCurrentDegrees()));

	delay(5000);
	// start the motor spinning again
	//motor1.SetSpeed(90);

 }


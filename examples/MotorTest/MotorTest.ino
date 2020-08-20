/*
 * This program will wait for the button to be pressed and then:
 *  command the left motor to spin at 60 rpm, 
 *  wait 5 seconds, 
 *  and then stop the motor. 
 * While the motor is spinning, the program will print out how much it has turned (in degrees).
 */

#include <Arduino.h>
#include <Esp32WifiManager.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <RBE1001Lib.h>

Motor motor1;
Motor motor2;
// pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
const int buttonPin = BOOT_FLAG_PIN ;

/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 */
void setup() 
{
  //delay(2000);

  // This will initialize the Serial as 115200 for prints
  Serial.begin(115200);
  Motor::allocateTimer(0);
  // pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
  motor1.attach(MOTOR1_PWM, MOTOR1_DIR, MOTOR1_ENCA, MOTOR1_ENCB);
  motor2.attach(MOTOR2_PWM, MOTOR2_DIR, MOTOR2_ENCA, MOTOR2_ENCB);
  //explicitly make the button pin an input and engage the internal pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);
}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() 
{
  //The following line will cause the program to wait indefinitely until the button is pressed
  while(digitalRead(buttonPin)) {} 

  Serial.println("Setting speeds");

  motor1.SetSpeed(180); 
  motor2.SetSpeed(-180); 

  uint32_t startTime = millis(); //note when the motor started


  while(millis() - startTime < 3000) //run for 3 seconds
  {
    Serial.print(motor1.getCurrentDegrees()); //motor1 position
    Serial.print('\t'); //TAB character
    Serial.print(motor2.getCurrentDegrees()); //motor2 position
    Serial.print('\n'); //newline character
  }
  	
	// stop the motor
	motor1.SetSpeed(0);
	motor2.SetSpeed(0);

  //The following line will cause the program to wait indefinitely until the button is pressed
  while(digitalRead(buttonPin)) {} 

  //motor1.SetEffort(-0.1); 
  motor2.SetEffort(0.3); 

  startTime = millis(); //note when the motor started

  while(millis() - startTime < 3000) //run for 3 seconds
  {
    Serial.print(motor1.getCurrentDegrees()); //motor1 position
    Serial.print('\t'); //TAB character
    Serial.print(motor2.getCurrentDegrees()); //motor2 position
    Serial.print('\n'); //newline character
  }
  	
	// stop the motor
	motor1.SetEffort(0);
	motor2.SetEffort(0);

  //The following line will cause the program to wait indefinitely until the button is pressed
  while(digitalRead(buttonPin)) {}
  motor1.StartMoveFor(720, -540); 
  motor2.MoveFor(-720, 540); 

  //The following line will cause the program to wait indefinitely until the button is pressed
  while(digitalRead(buttonPin)) {}

  motor1.StartMoveTo(0, -180); 
  motor2.MoveTo(0, 180); 

  while(digitalRead(buttonPin)) //run for 3 seconds
  {
    Serial.print(motor1.getCurrentDegrees()); //motor1 position
    Serial.print('\t'); //TAB character
    Serial.print(motor2.getCurrentDegrees()); //motor2 position
    Serial.print('\n'); //newline character
  }
}
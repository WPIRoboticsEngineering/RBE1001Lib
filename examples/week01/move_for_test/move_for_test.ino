/*
 * This program will wait for the button to be pressed and then:
 *  command the left motor to spin at 60 rpm, 
 *  wait 5 seconds, 
 *  and then stop the motor. 
 * While the motor is spinning, the program will print out how much it has turned (in degrees).
 */

#include <Arduino.h>
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

  motor1.MoveFor(360, 60); 
  motor2.MoveFor(360, 60); 

 }

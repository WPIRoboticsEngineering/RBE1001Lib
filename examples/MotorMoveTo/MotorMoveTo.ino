/*
 * This program will wait for the button to be pressed and then command the left motor to 
 * spin one rotation at 120 rpm. 
 * 
 * While the motor is spinning, the program will print out how much it has turned (in degrees).
 */

#include <Arduino.h>
#include <RBE1001Lib.h>

Motor motor_left;
Motor motor_right;

// pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
const int buttonPin = BOOT_FLAG_PIN;

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
  motor_left.attach(MOTOR_LEFT_PWM, MOTOR_LEFT_DIR, MOTOR_LEFT_ENCA, MOTOR_LEFT_ENCB);
  motor_right.attach(MOTOR_RIGHT_PWM, MOTOR_RIGHT_DIR, MOTOR_RIGHT_ENCA, MOTOR_RIGHT_ENCB);

  //explicitly make the button pin an input and engage the internal pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);
}

bool isSpinning = false;

/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() 
{
  //The following line will cause the program to wait indefinitely until the button is pressed
  if(!digitalRead(buttonPin) && !isSpinning) 
  { 
    motor_left.MoveTo(360, 120); //spin once at 120 degrees per second
    isSpinning = true;
  }

  Serial.print(motor_left.getCurrentDegrees()); //motor1 position
  Serial.print('\t'); //TAB character
  Serial.print(motor_right.getCurrentDegrees()); //motor2 position
  Serial.print('\n'); //newline character
}

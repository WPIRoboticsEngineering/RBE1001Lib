#include <Arduino.h>
#include <RBE1001Lib.h>

const int ledPin = 33;

// the setup routine runs once at the start of the program:
void setup() 
{                
  pinMode(ledPin, OUTPUT);         // initialize the digital pin as an output.
}

// the loop routine runs over and over:
void loop() 
{
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                  // wait for a second
  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                  // wait for a second
}

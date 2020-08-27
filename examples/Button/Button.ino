const int buttonPin = 0;     // the number of the pushbutton pin
const int ledPin =  33;      // the number of the LED pin

void setup() 
{
  pinMode(ledPin, OUTPUT);            // initialize the LED pin as an output
  pinMode(buttonPin, INPUT_PULLUP);   // initialize the pushbutton pin as an input (w/pullup)
}

void loop()
{
  // read the state of the pushbutton value:
  int buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is LOW (inverted logic!):
  if (buttonState == LOW) 
  {     
    // turn LED on:    
    digitalWrite(ledPin, HIGH);  
  } 
  else 
  {
    // turn LED off:
    digitalWrite(ledPin, LOW); 
  }
}

/*
 * A basic analog read example. This uses Arduino's pre-packaged analogRead() function, which 
 * doesn't use the ESP32's calibration system, but is good enough for our purposes here.
 */

#include <Arduino.h>
#include <RBE1001Lib.h>

const int photoresistorPin = 34;
const uint32_t LINE_SENSOR_INTERVAL = 10;

/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 */
void setup() 
{
  // This will initialize the Serial as 115200 for prints
  Serial.begin(115200);

  //Pins typically default to INPUT, but the code reaads easier if you are explicit:
  pinMode(photoresistorPin, INPUT);
}


/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started.
 */
void loop() 
{
  //delay(100);
  static uint32_t lastTime = 0;
  if(millis() - lastTime > LINE_SENSOR_INTERVAL)
  {
    int adcPhotoresistor = analogRead(photoresistorPin);
    Serial.print(millis());
    Serial.print('\t');
    Serial.println(adcPhotoresistor);

    lastTime += LINE_SENSOR_INTERVAL;
  }

}

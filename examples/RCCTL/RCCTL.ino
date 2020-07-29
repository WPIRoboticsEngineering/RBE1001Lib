#include <ESP32AnalogRead.h>
#include <Esp32WifiManager.h>
#include "DriveBase.h"

#include "Rangefinder.h"
#include "LineTrackSensor.h"
#include "Timer.h"
#include "RBE1001Lib.h"
#include <Esp32WifiManager.h>
#include "wifi/WifiManager.h"
#include "WebPage.h"

//Rangefinder rangefinder(FORWARD_ULTRASONIC_TRIG, FORWARD_ULTRASONIC_ECHO);
LineTrackSensor lineTrackSensor(LEFT_LINE_SENSE, RIGHT_LINE_SENSE);
DriveBase drive(MOTOR1_PWM, MOTOR2_PWM,MOTOR1_DIR,MOTOR2_DIR);

WebPage buttonPage;

WifiManager manager;

enum State {stopped, go} state;

/*
 * Each of these functions corresponds to a button on the web page. When the
 * associated button is pressed, the function is called. The functions can
 * contain any robot code, but in this case, I just set a new state and the
 * state machine in the loop() function starts doing whatever task you
 * selected
 */
void startMotor(String value) {
	Serial.println("GO!");
	Serial.println("Got from Server: "+value);
  state = go;
}

/*
 * Set up all the buttons that you will use for your final project
 * Each "newButton" call adds a button and connects the button to a function
 * that you add to the program like stopMotor()
 * that you can see above. At the end of each of those functions you need to
 * call "buttonPage.sendHTML()" to force the web page to redisplay after the
 * button is pressed.
 */
void setupButtons() {
  buttonPage.newButton("on", startMotor, "Motors On", "Turn On the motors");
}

Timer endTimer;   // end of a timed state timer
Timer dashboardUpdateTimer;  // times when the dashboard should update

/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 * In this example, it sets the Serial Console speed, initializes the web server,
 * sets up some web page buttons, resets some timers, and sets the initial state
 * the robot should start in
 */
void setup() {
  Serial.begin(115200);
  manager.setup();
	while (manager.getState() != Connected) {
		manager.loop();
	}
  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());


  buttonPage.initalize();
  setupButtons();
  endTimer.reset();     // reset the end of state timer
  dashboardUpdateTimer.reset(); // reset the dashbaord refresh timer
  state = stopped;   // initially, the robot is in the stopped state
}

/*
 * this is the state machine.
 * You can add additional states as desired. The switch statement will execute
 * the correct code depending on the state the robot is in currently.
 * For states that require timing, like turning and straight, they use a timer
 * that is zeroed when the state begins. It is compared with the number of
 * milliseconds the robot should reamain in that state.
 */
void runStateMachine() {
  switch(state) {
    case stopped:
      drive.motors(0, 0);
      break;
    case go:
      drive.motors(70, 70);
      break;
    }
}

/*
 * This function updates all the dashboard status that you would like to see
 * displayed periodically on the web page. You are free to add more values
 * to be displayed to help debug your robot program by calling the
 * "setValue" function with a name and a value.
 */
void updateDashboard() {
    // This writes values to the dashboard area at the bottom of the web page
    if (dashboardUpdateTimer.getMS() > 500) {
      //buttonPage.setValue("Rangefinder", rangefinder.getDistanceCM());
      buttonPage.setValue("Left linetracker", lineTrackSensor.getLeft());
      buttonPage.setValue("Right linetracker", lineTrackSensor.getRight());
      dashboardUpdateTimer.reset();
    }
}

/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started. In here we run the state machine, update the
 * dashboard data, and handle any web server requests.
 */
void loop() {
	manager.loop();
    runStateMachine();  // do a pass through the state machine
    updateDashboard();  // update the dashboard values
    buttonPage.handle();
 }

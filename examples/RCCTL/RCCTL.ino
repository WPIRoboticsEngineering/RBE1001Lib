#include <Arduino.h>
#include <RBE1001Lib.h>
#include "Motor.h"
#include "Rangefinder.h"
#include <ESP32Servo.h>
#include <ESP32AnalogRead.h>
#include <Esp32WifiManager.h>
#include "wifi/WifiManager.h"
#include "WebPage.h"
#include <Timer.h>
// https://wpiroboticsengineering.github.io/RBE1001Lib/classMotor.html
Motor motor1;
Motor motor2;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classRangefinder.html
Rangefinder rangefinder1;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classServo.html
Servo lifter;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classESP32AnalogRead.html
ESP32AnalogRead leftLineSensor;
ESP32AnalogRead rightLineSensor;
ESP32AnalogRead servoPositionFeedback;

WebPage buttonPage;

WifiManager manager;

enum State {
	stopped, go
} state;

Timer endTimer;   // end of a timed state timer
Timer dashboardUpdateTimer;  // times when the dashboard should update

/*
 * Each of these functions corresponds to a button on the web page. When the
 * associated button is pressed, the function is called. The functions can
 * contain any robot code, but in this case, I just set a new state and the
 * state machine in the loop() function starts doing whatever task you
 * selected
 */
void startMotor(String value) {
	Serial.println("GO!");
	Serial.println("Got from Server: " + value);
	state = go;
}
/*
 * Each of these functions corresponds to a button on the web page. When the
 * associated button is pressed, the function is called. The functions can
 * contain any robot code, but in this case, I just set a new state and the
 * state machine in the loop() function starts doing whatever task you
 * selected
 */
void stopMotor(String value) {
	Serial.println("STOP!");
	Serial.println("Got from Server: " + value);
	state = stopped;
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
	buttonPage.newButton("on", startMotor, "Motors On", "Run motors");
	buttonPage.newButton("off", stopMotor, "Motors Off",
			"Stop the motors");
}

/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 * In this example, it sets the Serial Console speed, initializes the web server,
 * sets up some web page buttons, resets some timers, and sets the initial state
 * the robot should start in
 */
void setup() {
	manager.setup();
	Motor::allocateTimer(0); // used by the DC Motors
	ESP32PWM::allocateTimer(1); // Used by servos
	// pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
	motor2.attach(MOTOR2_PWM, MOTOR2_DIR, MOTOR2_ENCA, MOTOR2_ENCB);
	motor1.attach(MOTOR1_PWM, MOTOR1_DIR, MOTOR1_ENCA, MOTOR1_ENCB);
	rangefinder1.attach(SIDE_ULTRASONIC_TRIG, SIDE_ULTRASONIC_ECHO);
	lifter.attach(SERVO_PIN);
	leftLineSensor.attach(LEFT_LINE_SENSE);
	rightLineSensor.attach(RIGHT_LINE_SENSE);
	servoPositionFeedback.attach(SERVO_FEEDBACK_SENSOR);
	lifter.write(0);

	while (manager.getState() != Connected) {
		manager.loop();
		delay(1);
	}

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
	switch (state) {
	case stopped:
		motor1.SetSpeed(0);
		motor2.SetSpeed(0);
		lifter.write(0);
		break;
	case go:
		motor1.SetSpeed(360);
		motor2.SetSpeed(360);
		lifter.write(180);
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
	if (dashboardUpdateTimer.getMS() > 100) {
		//buttonPage.setValue("Rangefinder", rangefinder.getDistanceCM());
		buttonPage.setValue("Left linetracker", leftLineSensor.readMiliVolts());
		buttonPage.setValue("Right linetracker",
				rightLineSensor.readMiliVolts());
		buttonPage.setValue("Ultrasonic",
				rangefinder1.getDistanceCM());
		buttonPage.setValue("Left Motor degrees",
						motor1.getCurrentDegrees());
		buttonPage.setValue("Right Motor degrees",
								motor2.getCurrentDegrees());
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
	if(manager.getState() == Connected)// only update if WiFi is up
		updateDashboard();  // update the dashboard values
}

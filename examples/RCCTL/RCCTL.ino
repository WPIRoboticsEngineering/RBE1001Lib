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
LeftMotor left_motor;
RightMotor right_motor;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classRangefinder.html
Rangefinder rangefinder1;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classServo.html
Servo lifter;
// https://wpiroboticsengineering.github.io/RBE1001Lib/classESP32AnalogRead.html
ESP32AnalogRead leftLineSensor;
ESP32AnalogRead rightLineSensor;
ESP32AnalogRead servoPositionFeedback;

WebPage control_page;

WifiManager manager;

Timer dashboardUpdateTimer; // times when the dashboard should update
/*
 * This is the standard setup function that is called when the ESP32 is rebooted
 * It is used to initialize anything that needs to be done once.
 * In this example, it sets the Serial Console speed, initializes the web server,
 * sets up some web page buttons, resets some timers, and sets the initial state
 * the robot should start in
 *
 * HOWTO enter passwords:
 *
 * The Detailed documentation is here: https://github.com/madhephaestus/Esp32WifiManager
 *
 * You will use the Serial Monitor to enter the name of the network and the password to use.
 * The ESP32 will store them for later.
 * First type the SSID you want and hit send
 * It will prompt you for a password, type that and hit send
 *
 * IN LAB use:
 * In the lab, you will want to use AP mode. To set the AP, type AP:myNetName
 * to set myNetName as the AP mode and hit send
 * the ESP will prompt you for a password to use, enter it and hit send
 *
 * The ESP will default to trying to connect to a network, then fail over to AP mode
 *
 * To make the ESP use AP mode by default on boot, change the line below manager.setup();
 * to manager.setupAP();
 *
 * To access the RC control interface for Station mode you will watch the serial monitor as the
 * ESP boots, it will print out the IP address. Enter that address in your computer or phones web browser.
 * Make sure your ESP and computer or phone are on the same network.
 *
 * To access teh RC Control interface in AP mode, connect to the ESP with either you phone or laptop
 * then open in a browser on that device and go to:
 *
 * http://192.168.4.1
 *
 * to access the control website.
 *
 * NOTE you can use this class in your final project code to visualize the state of your system while running wirelessly.
 */
int inc = 0;
void setup()
{
	manager.setup(); // Connect to an infrastructure network first, then fail over to AP mode
	//manager.setupAP();// Launch AP mode first, then fail over to connecting to a station
	while (manager.getState() != Connected)
	{
		manager.loop();
		delay(1);
	}
	ESP32PWM::allocateTimer(1); // Used by servos

	// pin definitions https://wpiroboticsengineering.github.io/RBE1001Lib/RBE1001Lib_8h.html#define-members
	rangefinder1.attach(SIDE_ULTRASONIC_TRIG, SIDE_ULTRASONIC_ECHO);
	lifter.attach(SERVO_PIN);
	leftLineSensor.attach(LEFT_LINE_SENSE);
	rightLineSensor.attach(RIGHT_LINE_SENSE);
	servoPositionFeedback.attach(SERVO_FEEDBACK_SENSOR);
	lifter.write(0);
	control_page.initalize();	  // Init UI after everything else.
	dashboardUpdateTimer.reset(); // reset the dashbaord refresh timer
}

/*
 * this is the state machine.
 * You can add additional states as desired. The switch statement will execute
 * the correct code depending on the state the robot is in currently.
 * For states that require timing, like turning and straight, they use a timer
 * that is zeroed when the state begins. It is compared with the number of
 * milliseconds the robot should reamain in that state.
 */
void runStateMachine()
{

	float left = (control_page.getJoystickX() - control_page.getJoystickY()) * 180;
	float right = (control_page.getJoystickX() + control_page.getJoystickY()) * 180;

	left_motor.setSpeed(left);
	right_motor.setSpeed(right);
	lifter.write(control_page.getSliderValue(0) * 180);
}

/*
 * This function updates all the dashboard status that you would like to see
 * displayed periodically on the web page. You are free to add more values
 * to be displayed to help debug your robot program by calling the
 * "setValue" function with a name and a value.
 */

uint32_t packet_old = 0;
void updateDashboard()
{
	// This writes values to the dashboard area at the bottom of the web page
	if (dashboardUpdateTimer.getMS() > 100)
	{

		control_page.setValue("Left linetracker", leftLineSensor.readMiliVolts());
		control_page.setValue("Right linetracker",
							  rightLineSensor.readMiliVolts());
		control_page.setValue("Ultrasonic",
							  rangefinder1.getDistanceCM());

		control_page.setValue("Simple Counter",
							  inc++);
		//if(control_page.getJoystickMagnitude()>0.1)
		//Serial.println("Joystick angle="+String(control_page.getJoystickAngle())+
		//		" magnitude="+String(control_page.getJoystickMagnitude())+
		//		" x="+String(control_page.getJoystickX())+
		//						" y="+String(control_page.getJoystickY()) +
		//						" slider="+String(control_page.getSliderValue(0)));

		control_page.setValue("packets from Web to ESP",
							  control_page.rxPacketCount);

		control_page.setValue("slider",
							  control_page.getSliderValue(0) * 100);

		control_page.setValue("Left Encoder Degrees", left_motor.getCurrentDegrees());
		control_page.setValue("Left Effort", (int)left_motor.getEffortPercent());
		control_page.setValue("Left Encoder Degrees/sec", left_motor.getDegreesPerSecond());

		control_page.setValue("Right Encoder Degrees", right_motor.getCurrentDegrees());
		control_page.setValue("Right  Effort", (int)right_motor.getEffortPercent());
		control_page.setValue("Free Ram", esp_get_free_heap_size());
		control_page.setValue("Right Encoder Degrees/sec", right_motor.getDegreesPerSecond());

		control_page.setValue("Simple Counter",
							  inc++);
		control_page.setValue("packets from Web to ESP",
							  control_page.rxPacketCount);
		control_page.setValue("packets to Web from ESP",
							  control_page.txPacketCount);

		dashboardUpdateTimer.reset();
	}
}

/*
 * The main loop for the program. The loop function is repeatedly called
 * once the ESP32 is started. In here we run the state machine, update the
 * dashboard data, and handle any web server requests.
 */

void loop()
{

	manager.loop();
	runStateMachine();					 // do a pass through the state machine
	if (manager.getState() == Connected) // only update if WiFi is up
		updateDashboard();				 // update the dashboard values
}

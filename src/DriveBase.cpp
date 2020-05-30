#include "DriveBase.h"
#include <Arduino.h>

DriveBase::DriveBase(int leftPWM, int rightPWM,int leftDIR, int rightDIR) {
  leftPinPWM = leftPWM;
  rightPinPWM = rightPWM;
  leftPinDIR=leftDIR;
  rightPinDIR=rightDIR;

}

void DriveBase::motors(int leftSpeed, int rightSpeed) {
//  dacWrite(leftPin, leftSpeed);
//  dacWrite(rightPin, rightSpeed);
}

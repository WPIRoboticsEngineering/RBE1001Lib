#include "DriveBase.h"
#include "arduino.h"

DriveBase::DriveBase(int left, int right) {
  leftPin = left;
  rightPin = right;
}

void DriveBase::motors(int leftSpeed, int rightSpeed) {
  dacWrite(leftPin, leftSpeed);
  dacWrite(rightPin, rightSpeed);
}

#include "LineTrackSensor.h"

LineTrackSensor::LineTrackSensor(int leftPin, int rightPin) {
    lineLeft.attach(leftPin);
    lineRight.attach(rightPin);
}

float LineTrackSensor::getLeft() {
    return lineLeft.readVoltage();
}

float LineTrackSensor::getRight() {
    return lineRight.readVoltage();
}

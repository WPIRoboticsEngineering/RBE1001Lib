#include "Arduino.h"
#include "Timer.h"

Timer::Timer() {
    reset();
}

void Timer::reset() {
    offset = millis();
}

unsigned long Timer::getMS() {
    return millis() - offset;
}
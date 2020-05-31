#pragma once
#include <Arduino.h>

class Timer {
    public:
        Timer();
        void reset();
        unsigned long getMS();
    private:
        unsigned long offset;
};

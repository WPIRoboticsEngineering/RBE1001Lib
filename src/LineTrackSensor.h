#pragma once
#include <ESP32AnalogRead.h>

class LineTrackSensor {
    public:
        LineTrackSensor(int left, int right);

        float getLeft();
        float getRight();
    private:
        ESP32AnalogRead lineLeft;
        ESP32AnalogRead lineRight;
};

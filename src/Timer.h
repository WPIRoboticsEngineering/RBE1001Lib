#pragma once

class Timer
{
public:
    Timer(unsigned long interval);
    bool isExpired();
    void reset();
    void reset(unsigned long newInterval);

private:
    unsigned long expiredTime;
    unsigned long timeInterval;
};

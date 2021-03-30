#include "Timer.h"
#include <Arduino.h>

/**
 * Create a timer that will expire every "interval"
 **/
Timer::Timer(unsigned long interval)
{
    expiredTime = millis() + interval;
    timeInterval = interval;
}

/**
 * Reset the timer to that the expired time is the current time + interval
 */
void Timer::reset()
{
    expiredTime = millis() + timeInterval;
}

/**
 * Change the timer interval to "NewInterval" then
 * reset the timer to that the expired time is the current time + interval
 */
void Timer::reset(unsigned long newInterval)
{
    timeInterval = newInterval;
    reset();
}

/**
 * Check if the timer is expired, that is the current time is past
 * the expired time.
 */
bool Timer::isExpired()
{
    if (millis() > expiredTime)
    {
        expiredTime = millis() + timeInterval;
        return true;
    }
    return false;
}

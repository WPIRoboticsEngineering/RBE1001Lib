#pragma once

/**
 * @brief A Timer that remembers a starting time and interval allow easy periodic events
 *
 * This class is for simple timing of things in the loop function where it will determine
 * if some amount of time has elapsed. When it is checked after the time has elapsed
 * then the timer is reset to simplify restarting the next interval.
 * Think of printing periodically.
 */
class Timer
{
public:
    /**
     * @brief Construct a new Timer object
     * 
     * @param interval 
     */
    Timer(unsigned long interval);
    /**
     * @brief Checks to see if the timer has expired.
     * 
     * @return true If the timer has expired, and the timer is reset for another interval.
     * @return false If the timer has not expired.
     */
    bool isExpired();
    /**
     * @brief Reset the timer to the current time + interval.
     * 
     */
    void reset();
    /**
     * @brief Reset the timer with a new interval
     * 
     * @param newInterval 
     */
    void reset(unsigned long newInterval);

private:
    unsigned long expiredTime;
    unsigned long timeInterval;
};

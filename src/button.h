//
//  button.h
//  
//
//  Created by Gregory C Lewin on 2/20/17.
//
//

/*
 * A simple button class with polling. Uses 'proper' debouncing with non-blocking timer
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button
{
private:
    enum BUTTON_STATE {BUTTON_STABLE, BUTTON_UNSTABLE};
    BUTTON_STATE state = BUTTON_STABLE;
    
    uint8_t buttonPin = -1;
    uint8_t activeState = LOW; //active LOW by default
    
    uint8_t stabButtonPos = HIGH; //most recent stable position
    uint8_t tempButtonPos = HIGH; //temporary position, may fluctuate due to bouncing
    
    uint32_t lastBounceTime = 0;
    uint32_t debouncePeriod = 10; // in ms
    
public:
    Button(uint8_t pin, uint32_t db = 10);
    void Init(bool usePullup = true);
    bool CheckButtonPress(void);
};

#endif /* button_h */

#include <button.h>

Button::Button(uint8_t pin, uint32_t db) 
{
    buttonPin = pin;
    debouncePeriod = db;
}

void Button::Init(bool usePullup)
{
    if(usePullup) pinMode(buttonPin, INPUT_PULLUP);
    else pinMode(buttonPin, INPUT);
    
    stabButtonPos = tempButtonPos = digitalRead(buttonPin);
}

bool Button::CheckButtonPress(void)
{
    bool retVal = false;
    uint8_t currButtonPos = digitalRead(buttonPin);
    
    if(tempButtonPos != currButtonPos)  //there's been a transistion, so start/continue debouncing
    {
        state = BUTTON_UNSTABLE;
        
        lastBounceTime = millis();      //start/restart the debouncing timer
        tempButtonPos = currButtonPos;  //keep track of the bouncing
    }

    if(state == BUTTON_UNSTABLE)
    {
        if(millis() - lastBounceTime >= debouncePeriod) //timer has expired
        {
            state = BUTTON_STABLE;
        }
    }

    if(state == BUTTON_STABLE)
    {
        if(stabButtonPos != tempButtonPos) //we have a transision
        {
            if(tempButtonPos == activeState) retVal = true;
            stabButtonPos = tempButtonPos;
        }
    }
    
    return retVal;
}

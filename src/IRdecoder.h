#include <Arduino.h>

/*
 * A class to interpret IR remotes with NEC encoding. NEC encoding sends four bytes:
 * 
 * [device ID, ~divice ID, key code, ~key code]
 * 
 * Sending the inverse allow for easy error checking (and reduces saturation in the receiver).
 * 
 * Codes are send in little endian; this library reverses upon reception, so the first bit received
 * is in the LSB of currCode. That means that the key code is found in bits [23..16] of currCode
 * 
 * https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
 * 
 * This does not interpret the codes into which key was pressed. That needs to be 
 * mapped on a remote by remote basis.
 */

/*
 * TO DO:
 * - (MED) Possibly indicate repeat code instead of just "refreshing" the latest code?
 * - (LOW) Consider a better way to indicate that a valid code has been entered;
 *   returning -1 is OK if we return a 16-bit number (so 255 is a valid code),
 *   but there may be a more elegant solution.
 * - (MED) Add ability to use PCINT library to this generic library. Use a derived class?
 */

class IRDecoder
{
private:
  enum IR_STATE
  {
    IR_READY,    //idle, returns to this state after you request a code
    IR_PREAMBLE, //received the start burst, waiting for first bit
    IR_REPEAT,   //received repeat code (part of NEC protocol); last code will be returned
    IR_ACTIVE,   //have some bits, but not yet complete
    IR_COMPLETE, //a valid code has been received
    IR_ERROR
  }; //an error occurred; won't return a valid code

  uint8_t irPin;

  IR_STATE state = IR_READY; //a simple state machine for managing reception

  volatile uint32_t lastReceiveTime = 0; //not really used -- could be used to sunset codes

  volatile uint32_t currCode = 0; //the most recently received valid code
  volatile uint8_t index = 0;     //for tracking which bit we're on

  volatile uint32_t fallingEdge = 0;
  volatile uint32_t risingEdge = 0;

  volatile uint32_t lastRisingEdge = 0; //used for tracking spacing between rising edges, i.e., bit value

public:
  IRDecoder(uint8_t pin) : irPin(pin) {}
  void init(void);           //call this in the setup()
  void handleIRsensor(void); //ISR

  uint32_t getCode(void) //returns the most recent valid code; returns zero if there was an error or nothing new
  {
    if (state == IR_COMPLETE || state == IR_REPEAT)
    {
      state = IR_READY;
      return currCode;
    }
    else
      return 0;
  }

  int16_t getKeyCode(bool acceptRepeat = false) //returns the most recent key code; returns -1 on error (not sure if 0 can be a code or not!!!)
  {
    if (state == IR_COMPLETE || (acceptRepeat == true && state == IR_REPEAT))
    {
      state = IR_READY;
      return (currCode >> 16) & 0x0ff; 
    }
    //else if(state == IR_ERROR) return currCode; //for debugging, if needed
    else
      return -1;
  }
};

class IRDecoder32U4 : public IRDecoder
{
  IRDecoder32U4(uint8_t pin) : IRDecoder(pin) {}
};

extern IRDecoder decoder;

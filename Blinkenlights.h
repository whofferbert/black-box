//
//
// blinkenlights, rgb led class
// common anode
//

#ifndef blinkenlights_h
#define blinkenlights_h

#include <Wire.h>

struct rgbPins {int r, g, b;};
struct rgbVals {int r, g, b;};

const int colorIncrementDelay = 0;

class rgbLED
{
public:
  // stuff
  rgbPins pins;
  rgbVals vals;

  // member funcs
  void roygbiv_cycle();
  rgbVals cycleLedRGB(rgbVals v);
  void RED();
  void ORANGE();
  void YELLOW();
  void GREEN();
  void BLUE();
  void INDIGO();
  void VIOLET();
  void on();
};

void rgbLED::roygbiv_cycle();

void rgbLED::on();

void rgbLED::RED();
void rgbLED::ORANGE();
void rgbLED::YELLOW();
void rgbLED::GREEN();
void rgbLED::BLUE();
void rgbLED::INDIGO();
void rgbLED::VIOLET();

rgbVals rgbLED::cycleLedRGB (rgbVals v);



#endif

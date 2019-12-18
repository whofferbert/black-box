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

void rgbLED::RED();

void rgbLED::ORANGE();

void rgbLED::YELLOW();

void rgbLED::GREEN();

void rgbLED::BLUE();

void rgbLED::INDIGO();

void rgbLED::VIOLET();

void rgbLED::on();

rgbVals rgbLED::cycleLedRGB (rgbVals v);



#endif

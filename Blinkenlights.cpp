//
//
// blinkenlights, rgb led class
// common anode
//

#include "Blinkenlights.h"
#include <Wire.h>

void rgbLED::roygbiv_cycle() {
  // stuff
  vals = cycleLedRGB(vals);
}

void rgbLED::RED() {
  vals = {0,255,255};
}

void rgbLED::ORANGE() {
  vals = {0,127,255};
}

void rgbLED::YELLOW() {
  vals = {0,0,255};
}

void rgbLED::GREEN() {
  vals = {255,0,255};
}

void rgbLED::BLUE() {
  vals = {255,255,0};
}

void rgbLED::INDIGO() {
  vals = {127,255,0};
}

void rgbLED::VIOLET() {
  vals = {0,255,0};
}


void rgbLED::on() {
  analogWrite(pins.r, vals.r);
  analogWrite(pins.g, vals.g);
  analogWrite(pins.b, vals.b);
}

// roll through the color spectrum
rgbVals rgbLED::cycleLedRGB (rgbVals v) {
  static int counter;
  // red to orange/yellow
  if (v.r == 0 && v.g > 0 && v.b == 255) {
    v.g--;
  // yellow to green
  } else if (v.r < 255 && v.g == 0 && v.b == 255) {
    v.r++;
  // green to blue
  } else if (v.r == 255 && v.g < 255 && v.b > 0) {
    v.g++;
    v.b--;
  // blue to violet
  } else if (v.r > 0 && v.g == 255 && v.b == 0) {
    v.r--;
  // violet to red
  } else if (v.r == 0 && v.g == 255 && v.b < 255) {
    v.b++;
  }
  return v;
}

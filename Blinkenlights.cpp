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

// roll through the color spectrum, with delays on ROYGBIV colors
rgbVals rgbLED::cycleLedRGB (rgbVals v) {
  //colorIncrementDelay
  static int redDelay;
  static int orangeDelay;
  static int yellowDelay;
  static int greenDelay;
  static int blueDelay;
  static int indigoDelay;
  static int violetDelay;

  // red to orange/yellow
  if (v.r == 0 && v.g > 0 && v.b == 255) {
    // reset previous counters
    violetDelay = 0;
    // pause on red
    if (v.g == 0 && redDelay < colorIncrementDelay) { 
      redDelay++;
    } else if (v.g == 127 && orangeDelay < colorIncrementDelay) {
      orangeDelay++;
    } else {
      v.g--;
    }
  // yellow to green
  } else if (v.r < 255 && v.g == 0 && v.b == 255) {
    redDelay = 0;
    orangeDelay = 0;
    if (v.r == 255 && yellowDelay < colorIncrementDelay) {
      yellowDelay++;
    } else {
      v.r++;
    }
  // green to blue
  } else if (v.r == 255 && v.g < 255 && v.b > 0) {
    yellowDelay = 0;
    if (v.g == 0 && greenDelay < colorIncrementDelay) {
      greenDelay++;
    } else {
      v.g++;
      v.b--;
    }
  // blue to indigo/violet
  } else if (v.r > 0 && v.g == 255 && v.b == 0) {
    greenDelay = 0;
    if (v.r == 255 && blueDelay < colorIncrementDelay) {
      blueDelay++;
    } else if (v.r == 127 && indigoDelay < colorIncrementDelay) {
      indigoDelay++;
    } else {
      v.r--;
    }
  // violet to red
  } else if (v.r == 0 && v.g == 255 && v.b < 255) {
    blueDelay = 0;
    indigoDelay = 0;
    if (v.b == 0 && violetDelay < colorIncrementDelay) {
      violetDelay++;
    } else {
      v.b++;
    }
  }
  return v;
}

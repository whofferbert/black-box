// testing grounds!
// here be dragons
// and whofferbert hacks, documented in ERRATA.txt
//
// okay so, general idea here is to take audio data from a guitar or bass.
// each string has it's own isolated pickup
// then turn the data from each individual string in to the base frequency and amplitude data.
// that data will then be analyzed / possibly modulated, and afterward,
// get translated into midi data, and sent over USB to whatever is powering the thing.
//
//  win.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <math.h>
#include <vector>
// https://github.com/wizard97/Embedded_RingBuf_CPP
#include "RingBufCPP.h"
#include "SingleNoteTracker.h"

// best current fix for both of: 
// undefined reference to `__exidx_end'
// undefined reference to `__exidx_start'
unsigned __exidx_start;
unsigned __exidx_end;
// re: https://forum.pjrc.com/threads/57192-Teensy-4-0-linker-issues-with-STL-libraries
// might break things that are already broken, but it's over my head

// master timer
unsigned long currentMillis = 0;

unsigned long previousLedMillis = 0;
unsigned long intervalLedMillis = 2;

unsigned long previousSerialMillis = 0;
unsigned long intervalSerialMillis = 1000;

//
//
// blinkenlights, 3x rgb led
//
//

struct rgbPins {int r, g, b;};

rgbPins led1 = {0, 1, 2};
rgbPins led2 = {4, 5, 6};
rgbPins led3 = {10, 11, 12};

struct rgbVals {int r, g, b;};

// green
rgbVals l1v = {255,0,255};
// blue
rgbVals l2v = {255,255,0};
// red
rgbVals l3v = {0,255,255};

void rgbIn(rgbPins l, rgbVals v) {
  analogWrite(l.r, v.r);
  analogWrite(l.g, v.g);
  analogWrite(l.b, v.b);
}

// roll through the color spectrum
rgbVals cycleLedRGB (rgbVals v) {
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

void cycleRGBs(){
  if (currentMillis - previousLedMillis > intervalLedMillis) {
    previousLedMillis = currentMillis;
    l1v = cycleLedRGB(l1v);
    l2v = cycleLedRGB(l2v);
    l3v = cycleLedRGB(l3v);
    rgbIn(led1,l1v);
    rgbIn(led2,l2v);
    rgbIn(led3,l3v);
  }
  // TODO the wiring means the audio signals 
  // pick up some weirdness when the PWM is 
  // changing a bunch
  // TODO this should be on a timestamped cycle, to release control back to the main loop
  // or maybe not delayed :|
  //delay(23);
  //delay(1);
}


//
//
// The rest is mostly just audio stuff
//
//


// GUItool: begin automatically generated code
AudioInputTDM            tdm1;           //xy=256.75,414.75
AudioAnalyzePeak         peak1;          //xy=666.75,233.75
AudioAnalyzeNoteFrequency notefreq1;      //xy=687.75,268.75
AudioAnalyzePeak         peak3;          //xy=687.75,399.75
AudioAnalyzeNoteFrequency notefreq6;      //xy=693.75,684.75
AudioAnalyzeNoteFrequency notefreq2;      //xy=702.75,357.75
AudioAnalyzePeak         peak2;          //xy=706.75,323.75
AudioAnalyzePeak         peak4;          //xy=708.75,467.75
AudioAnalyzeNoteFrequency notefreq3;      //xy=718.75,433.75
AudioAnalyzePeak         peak6;          //xy=718.75,637.75
AudioAnalyzePeak         peak5;          //xy=721.75,548.75
AudioAnalyzeNoteFrequency notefreq4;      //xy=723.75,511.75
AudioAnalyzeNoteFrequency notefreq5;      //xy=724.75,591.75
AudioMixer4              mixer2;         //xy=905.75,638.75
AudioMixer4              mixer1;         //xy=909.75,531.75
AudioOutputTDM           tdm2;           //xy=1206.75,406.75
AudioConnection          patchCord1(tdm1, 0, peak1, 0);
AudioConnection          patchCord2(tdm1, 0, notefreq1, 0);
AudioConnection          patchCord3(tdm1, 0, tdm2, 0);
AudioConnection          patchCord4(tdm1, 0, mixer1, 0);
AudioConnection          patchCord5(tdm1, 2, peak2, 0);
AudioConnection          patchCord6(tdm1, 2, notefreq2, 0);
AudioConnection          patchCord7(tdm1, 2, tdm2, 2);
AudioConnection          patchCord8(tdm1, 2, mixer1, 1);
AudioConnection          patchCord9(tdm1, 4, peak3, 0);
AudioConnection          patchCord10(tdm1, 4, notefreq3, 0);
AudioConnection          patchCord11(tdm1, 4, tdm2, 4);
AudioConnection          patchCord12(tdm1, 4, mixer1, 2);
AudioConnection          patchCord13(tdm1, 6, peak4, 0);
AudioConnection          patchCord14(tdm1, 6, notefreq4, 0);
AudioConnection          patchCord15(tdm1, 6, tdm2, 6);
AudioConnection          patchCord16(tdm1, 6, mixer1, 3);
AudioConnection          patchCord17(tdm1, 8, peak5, 0);
AudioConnection          patchCord18(tdm1, 8, notefreq5, 0);
AudioConnection          patchCord19(tdm1, 8, tdm2, 8);
AudioConnection          patchCord20(tdm1, 8, mixer2, 0);
AudioConnection          patchCord21(tdm1, 10, peak6, 0);
AudioConnection          patchCord22(tdm1, 10, notefreq6, 0);
AudioConnection          patchCord23(tdm1, 10, tdm2, 10);
AudioConnection          patchCord24(tdm1, 10, mixer2, 1);
AudioConnection          patchCord25(mixer2, 0, tdm2, 12);
AudioConnection          patchCord26(mixer2, 0, tdm2, 14);
AudioConnection          patchCord27(mixer1, 0, mixer2, 2);
AudioControlCS42448      cs42448_1;
// GUItool: end automatically generated code


// vectors of pointers to some of the above...
std::vector<AudioAnalyzeNoteFrequency *> freqs = {&notefreq1, &notefreq2, &notefreq3, &notefreq4, &notefreq5, &notefreq6};
std::vector<AudioAnalyzePeak *> peaks = {&peak1, &peak2, &peak3, &peak4, &peak5, &peak6};


// more global vars for the string trackers...
SingleNoteTracker string1;
SingleNoteTracker string2;
SingleNoteTracker string3;
SingleNoteTracker string4;
SingleNoteTracker string5;
SingleNoteTracker string6;

// vector for stringsNoteTracker pointers
std::vector<SingleNoteTracker *> strings = {&string1, &string2, &string3, &string4, &string5, &string6};

//
void serialPrinter() {
  if (currentMillis - previousSerialMillis > intervalSerialMillis) {
    previousSerialMillis = currentMillis;
    // possibly serial print things here
    Serial.println("got here");
  }
}

void setup() {
  // setup audio chip first, for as short a blip as possible
  // loooots of audio memory; thank you, teensy 4
  AudioMemory(512);
  cs42448_1.enable();
  cs42448_1.volume(1.0);
  cs42448_1.inputLevel(3.0);

  // turn on led1 after audio chip init
  rgbIn(led1,l1v);

  // debug/testing
  Serial.begin(9600);

  delay(1000);
  Serial.println("got past audio chip init and serial begin");
  delay(1000);

  // start frequency monitors
  for( AudioAnalyzeNoteFrequency * freq : freqs) {
    freq->begin(confidenceThreshold);
  }

  // led2 on after frequency analysis starts
  rgbIn(led2,l2v);

  delay(1000);
  Serial.println("got past frequency config");
  delay(1000);

  // setup string pointers... 
  unsigned char stringStepper = 0;
  for(SingleNoteTracker * string : strings) {
    string->name = stringStepper;
    string->freqRingBuf.add(0.0);
    string->peakRingBuf.add(0.0);
    string->velRingBuf.add(0);
    string->noteRingBuf.add(0);
    string->freqPointer = freqs[stringStepper]; 
    string->peakPointer = peaks[stringStepper];
    stringStepper++;
  }

  // led3 on after string setup
  rgbIn(led3,l3v);
  delay(1000);
  Serial.println("got past string setup");
  delay(1000);
}



void loop() {
  // update timer for things
  currentMillis = millis();

  // audio interaction
  // update strings signal data
  for(SingleNoteTracker * string : strings) {

    string->updateSignalData();

    if (string->hasAnythingChanged()) {
      // manage midi notes
      string->stringSignalToMidi();
    }
  }
  
  cycleRGBs();

  //serialPrinter();

  //commented for testing no midi connection yet
  while (usbMIDI.read()) {
    // ignore incoming messages, don't proliferate bugs
  }

}

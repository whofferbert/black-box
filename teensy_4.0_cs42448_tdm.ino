// here be dragons and whofferbert hacks, documented in ERRATA.txt
//
// okay so, general idea here is to take audio data from a guitar or bass.
// each string has it's own isolated pickup
// then turn the data from each individual string in to the base frequency and amplitude data.
// that data will then be analyzed / possibly modulated, and afterward,
// get translated into midi data, and sent over USB to whatever is powering the thing.

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
#include "Blinkenlights.h"

// best current fix for both of: 
// undefined reference to `__exidx_end'
// undefined reference to `__exidx_start'
unsigned __exidx_start;
unsigned __exidx_end;
// re: https://forum.pjrc.com/threads/57192-Teensy-4-0-linker-issues-with-STL-libraries
// might break things that are already broken, but it's over my head

// 
// Audio Chip Settings
// 
const float outputLevel = 1.0; // 0-1
const float inputLevel = 12.0; // 0-15

// freq closeness
const float freqProb = 0.13;

//
// timer stuff
//

unsigned long currentMillis = 0;

unsigned long previousLedMillis = 0;
//unsigned long intervalLedMillis = 100;
// 2-3 is really quick, nice lookin
const unsigned long intervalLedMillis = 11;

unsigned long previousSerialMillis = 0;
//unsigned long intervalSerialMillis = 30000;
const unsigned long intervalSerialMillis = 1000;

unsigned long previousAudioMillis = 0;
const unsigned long intervalAudioMillis = 3;

//
// LED stuff
//

rgbLED led1;
rgbLED led2;
rgbLED led3;

void cycleRGBs(){
  if (currentMillis - previousLedMillis > intervalLedMillis) {
    previousLedMillis = currentMillis;
    // increment counters
    led1.roygbiv_cycle();
    led2.roygbiv_cycle();
    led3.roygbiv_cycle();
    // push new colors to pins
    led1.on();
    led2.on();
    led3.on();
  }
}

//
// debugging
//
// TODO there should be a globally accessible ringbuf which gets written to
// and then this reads what's in the buffer, clearing it.
void serialPrinter() {
  if (currentMillis - previousSerialMillis > intervalSerialMillis) {
    previousSerialMillis = currentMillis;
    // possibly serial print things here
    //Serial.println("got here");
    int usage = AudioProcessorUsage();
    int maxUsage = AudioProcessorUsageMax();
    Serial.printf("Audio Processor: Current Usage: %d\tMax Usage: %d\n",  usage, maxUsage);
  }
}

//
// The rest is mostly just audio stuff
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


void setup() {
  // setup audio chip first, for as short a blip as possible
  // loooots of audio memory; thank you, teensy 4
  AudioMemory(512);
  cs42448_1.enable();
  cs42448_1.volume(outputLevel);
  cs42448_1.inputLevel(inputLevel);

  // setup LEDs
  led1.pins = {0, 1, 2};
  led2.pins = {4, 5, 6};
  led3.pins = {10, 11, 12};
  led1.BLUE();
  led2.YELLOW();
  led3.RED();

  // turn on led1 after audio chip init
  led1.on();

  // debug/testing
  Serial.begin(9600);

  delay(750);
  Serial.println("Audio chip init");
  delay(750);

  // start frequency monitors
  for( AudioAnalyzeNoteFrequency * freq : freqs) {
    // 0.15 is default
    freq->begin(freqProb);
  }

  // led2 on after frequency analysis starts
  led2.on();

  delay(750);
  Serial.println("Frequency config");
  delay(750);

  // setup string pointers... 
  // TODO maybe there should be a new/init method that does this?
  unsigned char stringStepper = 0;
  for(SingleNoteTracker * string : strings) {
    string->freqRingBuf.add(0.0);
    string->peakRingBuf.add(0.0);
    string->velRingBuf.add(0);
    string->noteRingBuf.add(0);
    string->name = stringStepper;
    string->freqPointer = freqs[stringStepper]; 
    string->peakPointer = peaks[stringStepper];
    stringStepper++;
  }

  // led3 on after string setup
  led3.on();

  delay(750);
  Serial.println("String setup");
  delay(750);
}


void updateStringData() {
  if (currentMillis - previousAudioMillis > intervalAudioMillis) {
    previousAudioMillis = currentMillis;
    for(SingleNoteTracker * string : strings) {
      //Serial.println("got here");
      string->updateSignalData();

      // 
      // TODO this is bad. it could be much better.
      // rather than a (any changes? do stuff) loop, there should be
      // a thing like 'process changes' where certain criteria are checked
      // and actions are taken appropriately. something functional, not all
      // this side-effecting bullshit we've got now.
      // 
  
      // manage midi notes
      if (string->hasAnythingChanged()) {
        string->stringSignalToMidi();
      }
    }
  }
}


void loop() {
  // update timer for things
  currentMillis = millis();

  // audio interaction
  // update strings signal data
  updateStringData();

  cycleRGBs();

  serialPrinter();

  //commented for testing no midi connection yet
  while (usbMIDI.read()) {
    // ignore incoming messages, don't proliferate bugs
  }

}

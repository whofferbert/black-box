// testing grounds!
// WIP
// here be dragons
// and whofferbert hacks

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <math.h>
#include <vector>
// https://github.com/wizard97/Embedded_RingBuf_CPP
#include "RingBufCPP.h"

// midi channel selection
// is this 0 or 1 based?
const int midiChannel = 1;

// midi note/peak buffer lengths.
// note: must be multiple of 2!
// maybe make this 16? 24?
const int noteRingBufferLength = 8;
const int peakRingBufferLength = 8;

// when to turn off notes because they fade out. 5 is arbitrary. 
const int midiMinimumVelocityThreshold = 1;

// note frequency confidence factor
// 0.15 = default. 0.11 = scrutiny
const float confidenceThreshold = 0.11;

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

std::vector<AudioAnalyzeNoteFrequency> freqs = {notefreq1, notefreq2, notefreq3, notefreq4, notefreq5, notefreq6};
std::vector<AudioAnalyzePeak> peaks = {peak1, peak2, peak3, peak4, peak5, peak6};
//
// TODO, WIP
//

//
// blinkenlights
//

long previousMillis = 0;

struct rgbPins {int r, g, b;};

rgbPins led1 = {0, 1, 2};
rgbPins led2 = {4, 5, 6};
rgbPins led3 = {10, 11, 12};

struct rgbVals {int r, g, b;};

rgbVals l1v = {255,0,255};
rgbVals l2v = {255,255,0};
rgbVals l3v = {0,255,255};

void rgbIn (rgbPins l, rgbVals v) {
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

//
//
// okay so, general idea here is to take audio data from a guitar or bass.
// each string has it's own isolated pickup
// then turn the data from each individual string in to the base frequency and amplitude data.
// that data will then be analyzed / possibly modulated, and afterward,
// get translated into midi data, and sent over USB to whatever is powering the thing.
//
//  win.


// bits of data to pass
struct signalData {float freq, peak;};
struct midiData {int note, velocity;};

// thanks, https://newt.phys.unsw.edu.au/jw/notes.html
int freqToMidiNote(float freq) {
  // 12 notes per scale, log base 2 (freq / reference freq) + ref freq midi number
  // 12 * (f/440) + 69
  return( int( roundf( 12.0 * log2f( freq / 440.0 ) ) + 69.0 ) );
}

int peakToMidiVelocity(float peak) {
  // convert float to 0-99 scale
  return(int(peak * 100.0));
}

void sendNoteOn(int note, int velocity) {
  // TODO option for always full velocity?
  // TODO velocity mod?
  //usbMIDI.sendNoteOn(60, 99, midiChannel);  // 60 = C4
  Serial.printf("Would have sent note %s ON, vel %s\n", note, velocity);
  //usbMIDI.sendNoteOn(note, velocity, midiChannel);
}

void sendNoteOff(int note) {
  //usbMIDI.sendNoteOff(60, 0, midiChannel);  // 60 = C4
  Serial.printf("Would have sent note %s OFF\n", note);
  //usbMIDI.sendNoteOff(note, 0, midiChannel);
}



// maybe investigate how to apply volume control/fade per active midi note, for decay
// TODO this class is out of hand. perhaps should be in another .cpp and .h file

// single string monitoring
class SingleNoteTracker
{
public:
  // ring buffers.
  // array of previous amplitudes
  //std::string name;
  bool noteIsOn = false;
  bool turnNoteOff = false;
  int currentNote = 0;
  int newNote = 0;
  int currentVel = 0;
  int newVel = 0;
  RingBufCPP<int, peakRingBufferLength> velRingBuf;
  // array of previous midi notes
  RingBufCPP<int, noteRingBufferLength> noteRingBuf;
  // keep track of current pitch/freq/note?
  RingBufCPP<float, noteRingBufferLength> freqRingBuf;
  RingBufCPP<float, peakRingBufferLength> peakRingBuf;
  AudioAnalyzeNoteFrequency * freqPointer;
  AudioAnalyzePeak * peakPointer;

  // member funcs;
  void updateSignalData();
  bool noteHasChanged();
  bool amplitudeChanged();
  bool hasAnythingChanged();
  void stringSignalToMidi();
  void test();
};

void SingleNoteTracker::test() {
  Serial.printf("Velocity: %d\tstuff\n", currentVel);
}

// sample the channel and add to ring buf
//
//
// TODO this is broken in general. requires more poking/understanding
// can't seem to be able to add things to the ring buffers properly
//
// tried with and without 'this->' signifiers, i don't think they are needed
// 
void SingleNoteTracker::updateSignalData() {
  int lastFreqIndex = freqRingBuf.numElements(); 
  int lastPeakIndex = peakRingBuf.numElements();
  //Serial.printf("Last Freq Index: %d\tLast Peak Index: %d\n", lastFreqIndex, lastPeakIndex);
  //float lastFreq = this->*freqRingBuf.peek(lastFreqIndex);
  //float lastPeak = this->*peakRingBuf.peek(lastPeakIndex);
  //float lastFreq = *freqRingBuf.peek(freqRingBuf.numElements() - 1);
  float lastFreq = 0.0;
  //float lastPeak = *peakRingBuf.peek(peakRingBuf.numElements() - 1);
  float lastPeak = 0.0;
  signalData tmpData;
  // this freqPointer also seems to not work...
  if (this->freqPointer->available() && this->peakPointer->available()) {
    //Serial.println("Got a freq and peak available.");
    float note = this->freqPointer->read();
    //float prob = freqPointer->probability();
    float peak = this->peakPointer->read();
    tmpData.freq = note;
    tmpData.peak = peak;
    // debugging/testing
    //int peakProb = peak * 100.0;
  } else {
    // TODO there's issues with this logic...
    // like what happens when a signal goes away
    // and the last signal we had was over the thresholds?
    tmpData.freq = lastFreq;
    tmpData.peak = lastPeak;
  }
  // TODO this never works, because we're never
  // adding things to the ringbufs
  int midiNote = freqToMidiNote(tmpData.freq);
  int midiVel = peakToMidiVelocity(tmpData.peak);
  this->freqRingBuf.add(tmpData.freq, true);
  this->peakRingBuf.add(tmpData.peak, true);
  this->noteRingBuf.add(midiNote, true);
  this->velRingBuf.add(midiVel, true);
}


// TODO funcs for comparing last data vs moving average in note buffer
// if there's a new note (reliably), turn old note off, new note on... how to weight that properly
bool SingleNoteTracker::noteHasChanged() {
  //int bufLen = noteRingBuf.numElements();
  int total;
  // 
  for (int i=0; i<noteRingBuf.numElements(); i++) {
    total += *noteRingBuf.peek(i);
  }
  int average = roundf(float(total) / float(noteRingBuf.numElements()));
  if (average != currentNote) {
    newNote = average;
    return(true);
  } else {
    return(false);
  }
}


bool SingleNoteTracker::amplitudeChanged() {
  bool changed = false;
  int total;
  for (int i=0; i<velRingBuf.numElements(); i++) {
    total += *velRingBuf.peek(i);
  }
  // if the amplitude jumps (threshold diff up) back up but for the same note, then turn old note off and back on.
  int average = roundf(float(total) / float(velRingBuf.numElements()));
  // TODO if amplitude jumps more than (percent? something?)
  if (average > currentVel) {
    newVel = average;
    changed = true;
  } else if (average <= midiMinimumVelocityThreshold) {
    newVel = average;
    turnNoteOff = true;
    changed = true;
  }

  return(changed);
}


bool SingleNoteTracker::hasAnythingChanged() {
  bool changed = false;
  if (noteHasChanged() || amplitudeChanged()) {
    changed = true;
  }
  return(changed);
}


void SingleNoteTracker::stringSignalToMidi() {
  // TODO based on things, turn off/on notes
  // TODO this might need to be different, like run every check func first, THEN step through note changes
  // handle turning notes off...
  bool noteWasTurnedOff = false;
  if (newVel > currentVel || newNote != currentNote || turnNoteOff == true) {
    // turn note off first
    sendNoteOff(currentNote);
    // mark that the note has been turned off
    noteIsOn = false;
    // mark that this fired?
    noteWasTurnedOff = true;
  }

  bool noteWasChanged = false;
  if (newNote != currentNote && noteIsOn == false) {
    // TODO an override for max velocity every time?
    // TODO sane to use newVel here?
    sendNoteOn(newNote, newVel);
    noteIsOn = true;
    noteWasChanged = true;
  } else if (newVel > currentVel && noteIsOn == false) {
    sendNoteOn(currentNote, newVel);
    noteIsOn = true;
    noteWasChanged = true;
  }

  // mmmmm
  if ( newNote != currentNote) {
    currentNote = newNote;
  }
  if ( newVel != currentVel) {
    currentVel = newVel;
  }

}


// more global vars for the string trackers...
SingleNoteTracker string1;
SingleNoteTracker string2;
SingleNoteTracker string3;
SingleNoteTracker string4;
SingleNoteTracker string5;
SingleNoteTracker string6;

//vector for stringsNoteTrackers
std::vector<SingleNoteTracker> strings = {string1, string2, string3, string4, string5, string6};




// below here mostly works

void setup() {
  // setup audio chip first, for as short a blip as possible
  // loooots of audio memory; thank you, teensy 4
  AudioMemory(512);
  cs42448_1.enable();
  cs42448_1.volume(1.0);
  cs42448_1.inputLevel(3.0);

  rgbIn(led1,l1v);
  // debug/testing
  Serial.begin(9600);

  delay(1000);
  Serial.println("got past audio chip init and serial begin");
  delay(1000);

  // LEDs
//  pinMode(0, OUTPUT);
//  pinMode(1, OUTPUT);
//  pinMode(2, OUTPUT);
//  pinMode(4, OUTPUT);
//  pinMode(5, OUTPUT);
//  pinMode(6, OUTPUT);
//  pinMode(10, OUTPUT);
//  pinMode(11, OUTPUT);
//  pinMode(12, OUTPUT);

  // start frequency monitors
  // this method does not work, and idk why
  //for( AudioAnalyzeNoteFrequency freq : freqs) {
  //  freq.begin(confidenceThreshold);
  //}
  //
  notefreq1.begin(confidenceThreshold);
  notefreq2.begin(confidenceThreshold);
  notefreq3.begin(confidenceThreshold);
  notefreq4.begin(confidenceThreshold);
  notefreq5.begin(confidenceThreshold);
  notefreq6.begin(confidenceThreshold);
  //

  rgbIn(led2,l2v);

  delay(1000);
  Serial.println("got past frequency config");
  delay(1000);

  // setup string pointers... 
  int stringStepper = 0;
  for(SingleNoteTracker string : strings) {
    Serial.printf("Setting up string %d\n", stringStepper + 1);
    string.freqRingBuf.add(0.0);
    string.peakRingBuf.add(0.0);
    string.velRingBuf.add(0);
    string.noteRingBuf.add(0);
    string.freqPointer = &freqs[stringStepper]; 
    string.peakPointer = &peaks[stringStepper];
    stringStepper++;
  }

  rgbIn(led3,l3v);
  delay(1000);
  Serial.println("got past string setup");
  delay(1000);
}

void cycleRGBs(){
  l1v = cycleLedRGB(l1v);
  l2v = cycleLedRGB(l2v);
  l3v = cycleLedRGB(l3v);
  rgbIn(led1,l1v);
  rgbIn(led2,l2v);
  rgbIn(led3,l3v);
  // TODO the wiring means the audio signals 
  // pick up some weirdness when the PWM is 
  // changing a bunch
  delay(25);
}

void loop() {
  // audio interaction
  // update strings signal data
  
  //Serial.println("Got to start of loop");
  int stringNumber = 1;
  for(SingleNoteTracker string : strings) {
    // this works...
    //Serial.printf("String: %d\tCurrent Note: %d\n", stringNumber, string.currentNote);
    // this works...
    //string.test();
    // TODO issues somewhere in below func...
    string.updateSignalData();
    //if (string.hasAnythingChanged()) {
      // manage midi notes
      // string.stringSignalToMidi();
    //}
    stringNumber++;
  }
  
  cycleRGBs();

  // more testing//
  if (notefreq1.available() && peak1.available()) {
    float freq = notefreq1.read();
    float peak = peak1.read();
    int note = freqToMidiNote(freq);
    int vel = peakToMidiVelocity(peak);
    Serial.printf("freq: %f\tnote: %d\tpeak: %f\tvel: %d\n", freq, note, peak, vel);
  }

  // wait more
  //delay(5000);

  //commented for testing no midi connection yet
  //while (usbMIDI.read()) {
    // ignore incoming messages, don't proliferate bugs
  //}

}

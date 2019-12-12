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
#include "RingBufCPP.h"
#include <vector>

// note frequency confidence factor
// 0.15 = default. 0.11 = scrutiny
const float confidenceThreshold = 0.11;

// midi channel selection
// is this 0 or 1 based?
const int midiChannel = 1;

// midi note/peak buffer lengths.
// note: must be multiple of 2!
const int noteRingBufferLength = 16;
const int peakRingBufferLength = 16;

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

//
// TODO, WIP
//




//
//
// okay so, general idea here is to take audio data from a guitar or bass.
// each string has it's own isolated pickup
// then turn the data from each individual string in to the base frequency and amplitude data.
// that data will then be analyzed / possibly modulated, and afterward,
// get translated into midi data, and sent over USB to whatever is powering the thing.
//
//  win.





// TODO
// there should be a thing in charge of midi note tracking... 
// like turning off old notes before moving on to new ones.
// how to manage a single stream of notes in a modular way ; apply that x6
// 

class NoteManager
{
public:
  // create a vector of note tracker objects
  std::vector<SingleNoteTracker> Strings;
  // if there's a new note (reliably), turn old note off, new note on
  // if the amplitude jumps (threshold diff up) back up but for the same note,
  //   turn old note off and back on.
  // if the note's amplitude has fallen below X threshold, turn note off
  // maybe investigate how to apply volume control/fade per active midi note, for decay
  
private:
};



// TODO
// we need some collection of logic to keep track of the most recent frequency and pitch
// data for a single (monophonic) source. 
// this should probably be a ring buffer or something
// and a set of moving averages?
// other fields for status? 
// freq rising, lowering, static, etc?
// 

// single string monitoring
class SingleNoteTracker
{
public:
  // ring buffers.
  // array of previous amplitudes
  RingBufCPP<float, peakRingBufferLength> peakRingBuf;
  // array of previous midi notes
  RingBufCPP<float, noteRingBufferLength> noteRingBuf;
  // keep track of current pitch/freq/note
  float currentNote;
  float currentPeak
  int midiNote;
  int midiVelocity;
  bool noteIsOn;
private:
  // things
};





// below here mostly works





// thanks, https://newt.phys.unsw.edu.au/jw/notes.html
int freqToMidiNote(float freq) {
  // 12 notes per scale, log base 2 (freq / reference freq) + ref freq midi number
  // 12 * (f/440) + 69
  return( int( roundf( 12.0 * log2f( freq / 440.0 ) ) + 69.0 ) );
}

void sendNoteOn(int note, int velocity) {
  // TODO option for always full velocity?
  // TODO velocity mod?
  //usbMIDI.sendNoteOn(60, 99, midiChannel);  // 60 = C4
  usbMIDI.sendNoteOn(note, velocity, midiChannel);
}

void sendNoteOff(int note) {
  //usbMIDI.sendNoteOff(60, 0, midiChannel);  // 60 = C4
  usbMIDI.sendNoteOff(note, 0, midiChannel);
}

// loop tasks for frequency/peak stuff.
// this should be made better
// instead of serial.printing things, it should send peak/freq data to
// the note manager object
// TODO should probably return a frequency and peak value
void signalCheck(AudioAnalyzeNoteFrequency * freqPointer, AudioAnalyzePeak * peakPointer) {
  if (freqPointer->available() && peakPointer->available()) {
    float note = freqPointer->read();
    float prob = freqPointer->probability();
    float peak = peakPointer->read();
    // debugging/testing
    int peakProb = peak * 100.0;
    Serial.printf("Note: %3.2f | Probability: %.2f | Peak: %d \n", note, prob, peak);
  }
}


// TODO these should output some a tuple of floats or something
// and then pass that to an updater program
void freqAndPeak() {
  signalCheck(&notefreq1, &peak1);
  signalCheck(&notefreq2, &peak2);
  signalCheck(&notefreq3, &peak3);
  signalCheck(&notefreq4, &peak4);
  signalCheck(&notefreq5, &peak5);
  signalCheck(&notefreq6, &peak6);
}



void setup() {
  // debug/testing
  Serial.begin(9600);
  // loooots of audio memory; thank you, teensy 4
  AudioMemory(512);
  cs42448_1.enable();
  cs42448_1.volume(1.0);
  // setup note buffers...
  //  
  notefreq1.begin(confidenceThreshold);
  notefreq2.begin(confidenceThreshold);
  notefreq3.begin(confidenceThreshold);
  notefreq4.begin(confidenceThreshold);
  notefreq5.begin(confidenceThreshold);
  notefreq6.begin(confidenceThreshold);
}


// for best effect with freqAndNote, make your terminal/monitor 
// a minimum of 62 chars wide, and as tall as you can.
void loop() {
  // audio interaction
  freqAndPeak();

  // TODO update something here with freqAndNote data.
  // mmmmmmmmmmmmmm
  // send midi notes.

  while (usbMIDI.read()) {
    // ignore incoming messages, don't proliferate bugs
  }

}

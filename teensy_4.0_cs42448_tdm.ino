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

// bits of data to pass
struct signalData {float freq, peak;};
struct midiData {int note, velocity;};

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

// loop tasks for frequency/peak stuff.
signalData signalCheck(AudioAnalyzeNoteFrequency * freqPointer, AudioAnalyzePeak * peakPointer) {
  signalData tmpData;
  if (freqPointer->available() && peakPointer->available()) {
    float note = freqPointer->read();
    float prob = freqPointer->probability();
    float peak = peakPointer->read();
    tmpData.freq = note;
    tmpData.peak = peak;
    // debugging/testing
    int peakProb = peak * 100.0;
    //Serial.printf("Note: %3.2f | Probability: %.2f | Peak: %d \n", note, prob, peak);
  } else {
    // wat do?
    tmpData.freq = 0.0;
    tmpData.peak = 0.0;
  }
  return(tmpData);
}



// TODO maybe a struct for holding current/last/timestamp/other data?

// single string monitoring
class SingleNoteTracker
{
public:
  // ring buffers.
  // array of previous amplitudes
  RingBufCPP<int, peakRingBufferLength> velRingBuf;
  // array of previous midi notes
  RingBufCPP<int, noteRingBufferLength> noteRingBuf;
  // keep track of current pitch/freq/note
  RingBufCPP<float, noteRingBufferLength> freqRingBuf;
  RingBufCPP<float, peakRingBufferLength> peakRingBuf;
  float currentNote;
  float currentPeak;
  AudioAnalyzeNoteFrequency * freqPointer;
  AudioAnalyzePeak * peakPointer;
  int midiNote;
  int midiVelocity;
  bool noteIsOn;
  // sample the channel and add to ring buf
  void updateSignalData(void) {
    signalData tmpData;
    tmpData = signalCheck(freqPointer, peakPointer);
    int midiNote = freqToMidiNote(tmpData.freq);
    int midiVel = peakToMidiVelocity(tmpData.peak);
    this->freqRingBuf.add(tmpData.freq, true);
    this->peakRingBuf.add(tmpData.peak, true);
    this->noteRingBuf.add(midiNote, true);
    this->velRingBuf.add(midiVel, true);
  }
private:
  // things
};


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

  string1.freqPointer = &notefreq1; 
  string1.peakPointer = &peak1; 
  string2.freqPointer = &notefreq2; 
  string2.peakPointer = &peak2; 
  string3.freqPointer = &notefreq3; 
  string3.peakPointer = &peak3; 
  string4.freqPointer = &notefreq4; 
  string4.peakPointer = &peak4; 
  string5.freqPointer = &notefreq5; 
  string5.peakPointer = &peak5; 
  string6.freqPointer = &notefreq6; 
  string6.peakPointer = &peak6; 
}


void loop() {
  // audio interaction
  // update strings signal data

  for(SingleNoteTracker string : strings) {
    string.updateSignalData();
    //string.stringSignalToMidi();
  }

  while (usbMIDI.read()) {
    // ignore incoming messages, don't proliferate bugs
  }

}

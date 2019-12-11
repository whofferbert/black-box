// testing grounds!
// Audio includes

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <math.h>

// note frequency confidence factor
const float confidenceThreshold = 0.11;

// midi channel selection
const int midiChannel = 1;

// midi note/peak tracking stuff
const int midiNoteBufferLength = 10;
const int midiPeakBufferLength = 10;

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
// okay so, general idea here is to take audio data from a guitar
// or bass, and turn that in to base frequency and amplitude data.
// that data will then be analyzed / possibly modulated, and afterward
// get translated into midi data.
// 

//
// there should be a thing in charge of midi note tracking... 
// like turning off old notes before moving on to new ones.
// how to manage a single stream of notes in a modular way ; apply that x6
// 

class NoteManager
{
//const int midiNoteBufferLength = 10;
//const int midiPeakBufferLength = 10;
public:
  // create a vector of objects
  // N by arrays 
  //  // what arrays
  // keep track of current pitch/freq/note
  // keep track of current amplitude
  // if there's a new note, turn note off
  // if the amplitude jumps back up but for the same note, turn note off and back on
  // if the note's amplitude has fallen below X threshold, turn note off
  // maybe fade volumes out?
private:
}

// 

class SingleNoteTracker
{
public:
  // single string monitoring
  // array of previous amplitudes
  // array of previous midi notes
  // 
private:
  // things
}



// thanks https://newt.phys.unsw.edu.au/jw/notes.html
int freqToMidiNote(float freq) {
  // 12 notes per scale, log base 2 (freq / reference freq) + ref freq midi number
  // 12 * (f/440) + 69
  float m = (12.0 * log2f(freq/440.0)) + 69.0;
  //float round = roundf(m);
  //return(int(round));
  return(int(roundf(m)));
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
void freqCheck(AudioAnalyzeNoteFrequency * freqPointer, AudioAnalyzePeak * peakPointer) {
  bool printed = false;  
  // TODO redo this? if freq and peak available?
  if (freqPointer->available()) {
    float note = freqPointer->read();
    float prob = freqPointer->probability();
    Serial.printf("Note: %3.2f | Probability: %.2f ----- ", note, prob);
    printed = true;
  }

  if (printed == true) {
    Serial.println("");
  }

  if (peakPointer->available() && printed == true) {
    float leftNumber = peakPointer->read();
    float rightNumber = leftNumber;
    int leftPeak = leftNumber * 30.0;
    int rightPeak = rightNumber * 30.0;
    int count;
    for (count=0; count < 30-leftPeak; count++) {
      Serial.print(" ");
    }
    while (count++ < 30) {
      Serial.print("<");
    }
    Serial.print("||");
    for (count=0; count < rightPeak; count++) {
      Serial.print(">");
    }
    while (count++ < 30) {
      Serial.print(" ");
    }
    Serial.print(leftNumber);
    Serial.print(", ");
    Serial.print(rightNumber);
    Serial.println();
  }
  
}

void freqAndNote() {
  freqCheck(&notefreq1, &peak1);
  freqCheck(&notefreq2, &peak2);
  freqCheck(&notefreq3, &peak3);
  freqCheck(&notefreq4, &peak4);
  freqCheck(&notefreq5, &peak5);
  freqCheck(&notefreq6, &peak6);
}



void setup() {
  Serial.begin(9600);
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
  // TODO should there be a more main kind of function here?
  // possibly something with a time counter/delay thingy?
  freqAndNote();

  // update something here with freqAndNote data.
  // send midi notes.

  while (usbMIDI.read()) {
    // ignore incoming messages
  }

}

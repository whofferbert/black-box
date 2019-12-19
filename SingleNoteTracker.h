// here be dragons, and whofferbert hacks, documented in ERRATA.txt
//
// okay so, general idea here is to take audio data from a guitar or bass.
// each string has it's own isolated pickup
// then turn the data from each individual string in to the base frequency and amplitude data.
// that data will then be analyzed / possibly modulated, and afterward,
// get translated into midi data, and sent over USB to whatever is powering the thing.

#ifndef SingleNoteTracker_H
#define SingleNoteTracker_H

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <math.h>
#include <vector>
// https://github.com/wizard97/Embedded_RingBuf_CPP
#include "RingBufCPP.h"

// note min/max
const unsigned char noteMin = 21;  // 5 string bass with a low A0
const unsigned char noteMax = 100; // high E7, 36th fret

// midi channel selection
// is this 0 or 1 based?
const int midiChannel = 1;

// local ring buffer lengths.
// note: must be multiple of 2!
// maybe make this 16? 24?
const int ringBufferLength = 8;
//const int ringBufferLength = 24;

// TODO not implemented yet
// when to turn off notes because they fade out.
const int midiMinimumVelocityThreshold = 2;

// note frequency confidence factor
// 0.15 = default. 0.11 = scrutiny
const float confidenceThreshold = 0.10;

// bits of data to pass
struct signalData {float freq, peak, weight;};

// a function to take a frequenct and return the closest midi note (0-127)
unsigned char freqToMidiNote(float freq);

// a function to take a float and translate it to 0-127
unsigned char peakToMidiVelocity(float peak);

// midi note on with velocity
void sendNoteOn(unsigned char note, unsigned char velocity);

// note off
void sendNoteOff(unsigned char note);


// maybe investigate how to apply volume control/fade per active midi note, for decay
// single string monitoring
class SingleNoteTracker
{
public:
  // ring buffers.
  // array of previous amplitudes
  unsigned char name;
  bool noteIsOn = false;
  bool turnNoteOff = false;
  unsigned char currentNote = 0;
  unsigned char newNote = 0;
  unsigned char currentVel = 0;
  unsigned char newVel = 0;
  RingBufCPP<unsigned char, ringBufferLength> velRingBuf;
  RingBufCPP<unsigned char, ringBufferLength> noteRingBuf;
  RingBufCPP<float, ringBufferLength> freqRingBuf;
  RingBufCPP<float, ringBufferLength> peakRingBuf;
  RingBufCPP<float, ringBufferLength> probRingBuf;
  AudioAnalyzeNoteFrequency * freqPointer;
  AudioAnalyzePeak * peakPointer;

  // member funcs;
  void updateSignalData();
  bool noteHasChanged();
  bool amplitudeChanged();
  bool hasAnythingChanged();
  void stringSignalToMidi();
  void sendNoteOn(unsigned char note, unsigned char velocity);
  void sendNoteOff(unsigned char note);

};

// sample the channel and add new data to ring buf
void SingleNoteTracker::updateSignalData();

// notes
void SingleNoteTracker::sendNoteOn(unsigned char note, unsigned char velocity);
void SingleNoteTracker::sendNoteOff(unsigned char note);

// funcs for comparing last data vs moving average in note buffer
// this seems extra broken ...
// if there's a new note (reliably), turn old note off, new note on... how to weight that properly
bool SingleNoteTracker::noteHasChanged();

// looking at peak changes
bool SingleNoteTracker::amplitudeChanged();

// looking for either peak or note changes
bool SingleNoteTracker::hasAnythingChanged();

// translating changes to midi notes
void SingleNoteTracker::stringSignalToMidi();

#endif

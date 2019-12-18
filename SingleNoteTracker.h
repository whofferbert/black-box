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

// midi channel selection
// is this 0 or 1 based?
const int midiChannel = 1;

// midi note/peak buffer lengths.
// note: must be multiple of 2!
// maybe make this 16? 24?
const int noteRingBufferLength = 8;
const int peakRingBufferLength = 8;

// when to turn off notes because they fade out.
const int midiMinimumVelocityThreshold = 2;

// note frequency confidence factor
// 0.15 = default. 0.11 = scrutiny
const float confidenceThreshold = 0.11;

// bits of data to pass
struct signalData {float freq, peak;};

// TODO the following four funcs might be better off in teh single note tracker class
// thanks, https://newt.phys.unsw.edu.au/jw/notes.html
unsigned char freqToMidiNote(float freq);


unsigned char peakToMidiVelocity(float peak);


void sendNoteOn(unsigned char note, unsigned char velocity);


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
  RingBufCPP<unsigned char, peakRingBufferLength> velRingBuf;
  // array of previous midi notes
  RingBufCPP<unsigned char, noteRingBufferLength> noteRingBuf;
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
};

// sample the channel and add to ring buf
// this seems to work now
void SingleNoteTracker::updateSignalData();


// TODO funcs for comparing last data vs moving average in note buffer
// this seems extra broken ...
// if there's a new note (reliably), turn old note off, new note on... how to weight that properly
bool SingleNoteTracker::noteHasChanged();


bool SingleNoteTracker::amplitudeChanged();


bool SingleNoteTracker::hasAnythingChanged();


void SingleNoteTracker::stringSignalToMidi();

#endif

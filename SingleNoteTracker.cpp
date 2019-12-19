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

// TODO the following four funcs might be better off in teh single note tracker class
// thanks, https://newt.phys.unsw.edu.au/jw/notes.html
unsigned char freqToMidiNote(float freq) {
  // 12 notes per scale, log base 2 (freq / reference freq) + ref freq midi number
  // 12 * (f/440) + 69
  unsigned char ret = roundf( 12.0 * log2f( freq / 440.0 ) ) + 69.0;
  return(ret);
}


unsigned char peakToMidiVelocity(float peak) {
  // convert float to 0-127 scale
  unsigned char ret = peak * 127.0;
  return(ret);
}


void SingleNoteTracker::sendNoteOn(unsigned char note, unsigned char velocity) {
  if (note > noteMax || note < noteMin) {
    return;
  }
  if (velocity > 127 || velocity < midiMinimumVelocityThreshold) {
    return;
  }
  Serial.printf("Would have sent note %d ON, vel %d\n", note, velocity);
  // TODO option for always full velocity?
  // TODO velocity mod?
  usbMIDI.sendNoteOn(note, velocity, midiChannel);
}


void SingleNoteTracker::sendNoteOff(unsigned char note) {
  if (noteIsOn == false) {
    return;
  }
  if (note > noteMax || note < noteMin) {
    return;
  }
  Serial.printf("Would have sent note %d OFF\n", note);
  usbMIDI.sendNoteOff(note, 0, midiChannel);
}

// sample the channel and add to ring buf
// this seems to work now
void SingleNoteTracker::updateSignalData() {
  int lastFreqIndex = freqRingBuf.numElements(); 
  int lastPeakIndex = peakRingBuf.numElements();
  float lastFreq = *freqRingBuf.peek(lastFreqIndex - 1);
  float lastPeak = *peakRingBuf.peek(lastPeakIndex - 1);
  // buffer is getting filled properly
  //Serial.printf("FreqIndex: %d\tPeakIndex: %d\n", lastFreqIndex, lastPeakIndex);
  // lastFreq lastPeak are correct
  //Serial.printf("Freq: %f\tPeak: %f\n", lastFreq, lastPeak);
  signalData tmpData;
  if (freqPointer->available() && peakPointer->available()) {
    float note = freqPointer->read();
    float peak = peakPointer->read();
    //float prob = freqPointer->probability();
    tmpData.freq = note;
    tmpData.peak = peak;
    //tmpData.weight = prob;
  //} else if (freqPointer->available() || peakPointer->available()) {
  } else if (freqPointer->available()) {
    // TODO there might be issues with this logic...
    // like what happens when a signal goes away
    // and the last signal we had was over the thresholds?
    //float prob = freqPointer->probability();
    tmpData.freq = lastFreq;
    tmpData.peak = lastPeak;
    //tmpData.weight = prob;
  } else {

  // TODO there needs to be better logic here perhaps
  // like if we don't get a signal, don't log a 0 immediately?
  // timer logic something something?

    tmpData.freq = 0.0;
    tmpData.peak = 0.0;
    //tmpData.weight = 0.0;
  }

  unsigned char midiNote = freqToMidiNote(tmpData.freq);
  unsigned char midiVel = peakToMidiVelocity(tmpData.peak);

  //if (name == 1) {
  //  Serial.printf("Freq: %f\tPeak: %f\tMidi Note:%d\tVel: %d\n", tmpData.freq, tmpData.peak, midiNote, midiVel);
  //}

  // this has to be working because of the above peek logic working.
  freqRingBuf.add(tmpData.freq, true);
  peakRingBuf.add(tmpData.peak, true);
  //probRingBuf.add(tmpData.weight, true);
  noteRingBuf.add(midiNote, true);
  velRingBuf.add(midiVel, true);
}


// TODO funcs for comparing last data vs moving average in note buffer
// this seems extra broken ...
// if there's a new note (reliably), turn old note off, new note on... how to weight that properly
bool SingleNoteTracker::noteHasChanged() {
  int bufLen = noteRingBuf.numElements();
  unsigned char total = 0;
  for (int i=0; i< bufLen; i++) {
    // notebuf
    unsigned char noteVal = *noteRingBuf.peek(i);
    //if (name == 0) {
    //  Serial.printf("%d %d;\t", i, noteVal);
    //}
    total += noteVal;
  }
  unsigned char average = total / bufLen;
  //if (name == 1) {
  //  Serial.printf("total: %d\tcurrentNote: %d\tBuffer Average: %d\n", total , currentNote, average);
  //}
  if (average != currentNote) {
    //Serial.printf("Want to turn on note %d\n", average);
    newNote = average;
    return(true);
  } else {
    // testing?
    newNote = currentNote;
    return(false);
  }
}


bool SingleNoteTracker::amplitudeChanged() {
  bool changed = false;
  int bufLen = velRingBuf.numElements();
  unsigned char total = 0;
  for (int i=0; i< bufLen; i++) {
    total += *velRingBuf.peek(i);
    //if (name == 0) {
    //  Serial.printf("%d %d;\t", i, noteVal);
    //}
  }
  // if the amplitude jumps (threshold diff up) back up but for the same note, then turn old note off and back on.
  unsigned char average = total / bufLen;
  //if (name == 0) {
  //  Serial.printf("total: %d\tcurrentVel: %d\tBuffer Average: %d\n", total , currentVel, average);
  //}
  // TODO if amplitude jumps more than (percent? something?)
  if (average > currentVel) {
    //Serial.printf("Want to turn resend note %d\n", average);
    newVel = average;
    changed = true;
  } else if (average <= midiMinimumVelocityThreshold) {
    newVel = average;
    turnNoteOff = true;
    changed = true;
  } else {
    //newVel = average;
  }

  return(changed);
}


bool SingleNoteTracker::hasAnythingChanged() {
  bool changed = false;
  if (noteHasChanged()) {
    changed = true;
  } else if (amplitudeChanged()) {
    changed = true;
  }
  return(changed);
}


// 
// 
//  The most finesse will probably be required here
// 
// 

// TODO currently we're getting lots of note off signals

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
  if (noteWasChanged == true) {
    if ( newNote != currentNote) {
      currentNote = newNote;
    }
    if ( newVel != currentVel) {
      currentVel = newVel;
    }
  }
}


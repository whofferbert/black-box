// here be dragons and whofferbert hacks, documented in ERRATA.txt
//
// okay so, general idea here is to take audio data from a guitar or bass.
// each string has it's own isolated pickup
// then turn the data from each individual string in to the base frequency and amplitude data.
// that data will then be analyzed / possibly modulated, and afterward,
// get translated into midi data, and sent over USB to whatever is powering the thing.


//
// GENERAL IDEAS, since we migth not be able to sample things hyper frequently and current logic is borken.
//
// string decay is generally a relationship between string weight and pitch, based on velocity
// low notes ring longer than high notes
// might be able to include some extra sanity for things by having a table of what frequencies would decay over what rates
// and then apply those rates to whatever velocity things start at
// new note E2 with velocity 50? it'll ring for X ms. add a timer check and if >, then turn note off
// new note E6 with velocity 10? Y ms, etc... picture gotten

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

// TODO 
// should each string be on it's own midi channel for note on/off separation?

// TODO the following four funcs might be better off in teh single note tracker class
// thanks, https://newt.phys.unsw.edu.au/jw/notes.html
unsigned char freqToMidiNote(float freq) {
  // 12 notes per scale, log base 2 (freq / reference freq) + ref freq midi number
  // 12 * (f/440) + 69
  unsigned char ret = roundf( 12.0 * log2f( freq / 440.0 ) ) + 69.0;
  return(ret);
}


signed char peakToMidiVelocity(float peak) {
  // convert float to 0-127 scale
  signed char num = peak * 127.0;
  signed char ret;
  if (num < 127 && num > midiMinimumVelocityThreshold) {
    ret = 127;
  } else {
    ret = 0;
  }
  return(ret);
  
}

//
// you can Serial.print to debug the midi, maybe better to watch 
// aseqdump -p (port of this thing)
//

void SingleNoteTracker::sendNoteOn(unsigned char note, signed char velocity) {
  if (note > noteMax || note < noteMin) {
    return;
  }
  if (velocity == 0) {
    return;
  }
  //if (velocity > 127 || velocity < midiMinimumVelocityThreshold) {
  //  return;
  //}
  Serial.printf("Would have sent note %d ON, vel %d\n", note, 127);
  // TODO option for always full velocity?
  // TODO velocity mod?
  usbMIDI.sendNoteOn(note, 127, midiChannel);
  noteIsOn = true;
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
  noteIsOn = false;
  // TODO this logic is probably unnecessary
  turnNoteOff = false;
}

// sample the channel and add to ring buf
// this seems to work now
// TODO .... if there's nothing available, don't do anything with
// the ring buffers?
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
  bool ringBufAdd = false;
  if (freqPointer->available() && peakPointer->available()) {
    float note = freqPointer->read();
    float peak = peakPointer->read();
    float prob = freqPointer->probability();
    tmpData.freq = note;
    tmpData.peak = peak;
    tmpData.weight = prob;
    ringBufAdd = true;
  } else if (peakPointer->available()) {
    float note = lastFreq;
    float prob = 0.5; // 50/50 might be generous
    float peak = peakPointer->read();
    tmpData.freq = note;
    tmpData.peak = peak;
    tmpData.weight = prob;
    ringBufAdd = true;
  } else {
    Serial.println("Nothing available");
  }

  // else turn note off?

  if (ringBufAdd == true) {
    unsigned char midiNote = freqToMidiNote(tmpData.freq);
    signed char midiVel = peakToMidiVelocity(tmpData.peak);

    /* debugging... most of the time it'll be a freq guess at .5 prob. 
    if (name == 1 && midiVel != 0) {
      Serial.printf("Freq: %f\tPeak: %f\tMidi Note:%d\tVel: %d\tProb: %f\n", tmpData.freq, tmpData.peak, midiNote, midiVel, tmpData.weight);
    }
    */
    //
    if (name == 1 && midiVel != 0 && tmpData.weight != 0.5) {
      Serial.printf("\nFreq: %f\tPeak: %f\tMidi Note:%d\tVel: %d\tProb: %f\n", tmpData.freq, tmpData.peak, midiNote, midiVel, tmpData.weight);
    }
    if (name == 1 && midiVel != 0 && tmpData.weight == 0.5) {
      Serial.print(".");
    }
    //

    // this has to be working because of the above peek logic working.
    freqRingBuf.add(tmpData.freq, true);
    peakRingBuf.add(tmpData.peak, true);
    //probRingBuf.add(tmpData.weight, true);
    noteRingBuf.add(midiNote, true);
    velRingBuf.add(midiVel, true);
    //velRingBuf.add(midiVel * velocityMod, true);
    // rely on velocity note offing?
  }

}





// TODO funcs for comparing last data vs moving average in note buffer
// this seems extra broken ...
// if there's a new note (reliably), turn old note off, new note on... how to weight that properly
bool SingleNoteTracker::noteHasChanged() {
  int bufLen = noteRingBuf.numElements();
  unsigned char tmpNote = *noteRingBuf.peek(bufLen - 1);
  if (tmpNote != currentNote) {
    newNote = tmpNote;
    return(true);
  } else {
    // = tmpNote;
    return(false);
  }
  /*
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
  */
}


bool SingleNoteTracker::amplitudeChanged() {
  bool changed = false;
  int bufLen = velRingBuf.numElements();
  //signed char lastPeak = *velRingBuf.peek(bufLen - 1);
  
  signed char total = 0;
  for (int i=0; i< bufLen; i++) {
    total += *velRingBuf.peek(i);
    //if (name == 0) {
    //  Serial.printf("%d %d;\t", i, noteVal);
    //}
  }
  // if the amplitude jumps (threshold diff up) back up but for the same note, then turn old note off and back on.
  signed char average = total / bufLen;
  

  //if (name == 0) {
  //  Serial.printf("total: %d\tcurrentVel: %d\tBuffer Average: %d\n", total , currentVel, average);
  //}
  // TODO if amplitude jumps more than (percent? something?)
  //if (lastPeak > currentVel) {
  if (average > currentVel) {
    //Serial.printf("Want to turn resend note %d\n", average);
    //newVel = lastPeak;
    newVel = average;
    changed = true;
  //} else if (lastPeak <= midiMinimumVelocityThreshold) {
  //  newVel = lastPeak;
  // TODO the following is broken, at least by merit of turnNoteOff being problematic
  } else if (average <= midiMinimumVelocityThreshold) {
    newVel = average;
    turnNoteOff = true;
    changed = true;
  } else {
    //newVel = average;
  }

  return(changed);
}


// TODO this is music... should we assume there will always be change?

bool SingleNoteTracker::hasAnythingChanged() {
  if (noteHasChanged() || amplitudeChanged()) {
    return(true);
  } else {
    return(false);
  }
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
  // this fires too much
  // handle turning notes off...
  if (newVel > currentVel) sendNoteOff(currentNote);
  if (newNote != currentNote) sendNoteOff(currentNote);
  // TODO this seems broken!!
  //if (turnNoteOff == true) sendNoteOff(currentNote);

  bool noteWasChanged = false;
  if (newNote != currentNote && noteIsOn == false) {
    // TODO an override for max velocity every time?
    // TODO sane to use newVel here?
    sendNoteOn(newNote, newVel);
    noteWasChanged = true;
  } else if (newVel > currentVel && noteIsOn == false) {
    sendNoteOn(currentNote, newVel);
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


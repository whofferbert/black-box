// testing grounds!
// Audio includes

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// note frequency confidence factor
const float confidenceThreshold = 0.15;

// GUItool: begin automatically generated code
AudioInputTDM            tdm1;           //xy=256.75,436.75
AudioAnalyzePeak         peak1;          //xy=508.75,227.75
AudioAnalyzePeak         peak3;          //xy=526.75,395.75
AudioAnalyzeNoteFrequency notefreq1;      //xy=527.75,263.75
AudioAnalyzePeak         peak5;          //xy=527.75,552.75
AudioAnalyzeNoteFrequency notefreq3;      //xy=532.75,429.75
AudioAnalyzeNoteFrequency notefreq2;      //xy=533.75,350.75
AudioAnalyzePeak         peak4;          //xy=536.75,473.75
AudioAnalyzeNoteFrequency notefreq6;      //xy=539.75,688.75
AudioAnalyzePeak         peak2;          //xy=541.75,315.75
AudioAnalyzePeak         peak6;          //xy=543.75,635.75
AudioAnalyzeNoteFrequency notefreq5;      //xy=553.75,596.75
AudioAnalyzeNoteFrequency notefreq4;      //xy=554.75,511.75
AudioMixer4              mixer2;         //xy=745.75,581.75
AudioMixer4              mixer1;         //xy=747.75,502.75
AudioOutputTDM           tdm2;           //xy=978.75,350.75
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
AudioConnection          patchCord26(mixer1, 0, mixer2, 2);
AudioControlCS42448      cs42448_1;      //xy=595.75,761.75
// GUItool: end automatically generated code




// loop tasks for frequency stuff.
// this should be made better
void freqAndNote() {
  bool printed = false;  
  if (notefreq1.available()) {
    float note = notefreq1.read();
    float prob = notefreq1.probability();
    Serial.printf("Note: %3.2f | Probability: %.2f ----- ", note, prob);
    printed = true;
  }

  if (printed == true) {
    Serial.println("");
  }

  if (peak1.available() && printed == true) {
    float leftNumber = peak1.read();
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



void setup() {
  Serial.begin(9600);
  AudioMemory(90);
  cs42448_1.enable();
  cs42448_1.volume(0.5);
  mixer1.gain(0, 0.05);
  mixer1.gain(1, 0.05);
  mixer1.gain(2, 0.05);
  mixer1.gain(3, 0.05);
  mixer2.gain(0, 0.05);
  mixer2.gain(1, 0.05);
  mixer2.gain(2, 0.5);
  
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
  freqAndNote();
}

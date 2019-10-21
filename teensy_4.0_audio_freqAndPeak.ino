// testing ground

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

//#include "ILI9341_t3.h"

//apparently causes problems for other things that use
// attachInterrupt()
//#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=288.23333740234375,264.23333740234375
AudioAnalyzePeak         peak1;          //xy=507.23333740234375,114.23332977294922
//AudioAnalyzeFFT1024      fft1024_1;      //xy=516.2333374023438,191.23333740234375
AudioAnalyzeNoteFrequency notefreq1;      //xy=514.2333374023438,210.23333740234375
AudioOutputI2S           i2s1;           //xy=668.2333374023438,299.23333740234375
AudioConnection          patchCord1(i2s2, 1, i2s1, 0);
AudioConnection          patchCord2(i2s2, 1, i2s1, 1);
AudioConnection          patchCord3(i2s2, 1, peak1, 0);
AudioConnection          patchCord4(i2s2, 1, notefreq1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=477.23333740234375,486.23333740234375
// GUItool: end automatically generated code



// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.5 & 3.6 SD card
//#define SDCARD_CS_PIN    BUILTIN_SDCARD
//#define SDCARD_MOSI_PIN  11  // not actually used
//#define SDCARD_SCK_PIN   13  // not actually used

// Use these for the SD+Wiz820 or other adaptors
//#define SDCARD_CS_PIN    4
//#define SDCARD_MOSI_PIN  11
//#define SDCARD_SCK_PIN   13

//ILI9341 attached
#define TFT_CS   -1
#define TFT_DC   36
#define TFT_MOSI 35
#define TFT_CLK  37
#define TFT_RST  -1
#define TFT_MISO 34
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
//ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// display updating
unsigned long previousDisplayUpdate = 0;
unsigned long delayMillisInterval = 1000;
uint8_t rotation = 3;
bool displayNeedsUpdate = true;

/*
 * 
 * menu stuff
 *
 */

char *mainMenu[] = {
  "Stuff and things!",
  "foo & bar",
  "baz & qux",
  "chumblespuzz + qix + fizz",
  "another one",
  "do it again",
  "Captain",
  "Jenkins"
};

uint8_t mainMenuSize = 8;

uint8_t mainMenuPosition = 0;





// hardware setup
// Encoder name(DT, CLK)
Encoder leftRE(28,31);
Encoder middleRE(32, 30);
Encoder rightRE(5,3);

unsigned long previousREUpdate = 0;
unsigned long REMillisInterval = 50;

long leftREPos = 0;
long middleREPos = 0;
long rightREPos = 0;


/*
 * 
 * Functions below! setup and etcetera
 * 
 */


void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(rotation);
  AudioMemory(90);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  welcomeScreen();
  // set pin interrupts
  
  notefreq1.begin(.15);
  delay(3250);
  pedalMenu();
}

void welcomeScreen() {
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  //unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED);  
  tft.setTextSize(4);
  tft.println("Audio Hacker");
  tft.setTextColor(ILI9341_YELLOW); 
  tft.setTextSize(2);
  tft.println("A pedal full of nonesense");
  tft.setTextColor(ILI9341_GREEN);    
  tft.setTextSize(2);
  tft.println("by William Hofferbert");
  tft.println();
  Serial.print("welcomeScreen microseconds: ");
  Serial.println(micros() - start);
}

void updatePedalMenu() {
  static int printCounts;
  unsigned long start = micros();
  menuHeader();
  tft.setTextSize(3);      
  tft.setTextColor(ILI9341_GREEN);
  for (int i=0; i < mainMenuSize; i++) {
    if (i == mainMenuPosition) {
      tft.setTextColor(ILI9341_WHITE);
      tft.println(mainMenu[i]);
      tft.setTextColor(ILI9341_GREEN);    
    } else {
      tft.println(mainMenu[i]);
    }
  }
  if (printCounts < 20) {
    Serial.print("updatePedalMenu microseconds: ");
    Serial.println(micros() - start);
    printCounts+=1;
  }
}

void placeHolderMenuHeader() {
  tft.setCursor(0, 0);
  tft.setTextSize(5);      
  tft.println("");
}



void menuHeader() {
  tft.setCursor(0, 0);
  tft.setTextSize(5);      
  tft.println("Main Menu");
}

void pedalMenu() {
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  menuHeader();
  tft.setTextSize(3);      
  tft.setTextColor(ILI9341_GREEN);
  for (int i=0; i < mainMenuSize; i++) {
    if (i == mainMenuPosition) {
      tft.setTextColor(ILI9341_WHITE);
      tft.println(mainMenu[i]);
      tft.setTextColor(ILI9341_GREEN);    
    } else {
      tft.println(mainMenu[i]);
    }
  }
  Serial.print("pedalMenu microseconds: ");
  Serial.println(micros() - start);
}

void updateRotaryEncoders() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousREUpdate > REMillisInterval) {
    previousREUpdate = currentMillis;
    long newLeft, newRight, newMiddle;
    newLeft = leftRE.read();
    newMiddle = middleRE.read();
    newRight = rightRE.read();
    if (newLeft != leftREPos) {
      leftREPos = newLeft;
      Serial.print("Left RE new position: ");
      Serial.println(newLeft);
    }
    if (newMiddle != middleREPos) {
      middleREPos = newMiddle;
      Serial.print("Middle RE new position: ");
      Serial.println(newMiddle);
    }
    if (newRight != rightREPos) {
      rightREPos = newRight;
      Serial.print("Right RE new position: ");
      Serial.println(newRight);
    }
  }
}

void displayIntervalTest() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousDisplayUpdate > delayMillisInterval) {
    previousDisplayUpdate = currentMillis;
    if (mainMenuPosition < (mainMenuSize - 1)) {
      mainMenuPosition+=1;
    } else {
      mainMenuPosition = 0;
    }
    //Serial.print("got here, position is");
    //Serial.println(mainMenuPosition);
    displayNeedsUpdate = true; 
  }
  
}

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



// for best effect make your terminal/monitor a minimum of 62 chars wide and as high as you can.
void loop() {
  freqAndNote();
  
  //update screen if we need to
  if (displayNeedsUpdate == true) {
    //Serial.println("updating display ...");
    // run updates to display
    updatePedalMenu();
    displayNeedsUpdate = false;
  }

  displayIntervalTest();
  updateRotaryEncoders();
}

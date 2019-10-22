// testing grounds
// Audio includes

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// ILI9341 includes

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

//#include "ILI9341_t3.h"

// hardware includes

// apparently ENCODER_OPTIMIZE_INTERRUPTS causes problems 
// for other things that use attachInterrupt()?
// unless Paul fixed it
//#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

// hardware setup

// Rotary Encoder Switches

#define LEFT_RE_SWITCH_PIN   29
#define MIDDLE_RE_SWITCH_PIN 33
#define RIGHT_RE_SWITCH_PIN  4

Bounce leftRESwitch = Bounce();
Bounce middleRESwitch = Bounce();
Bounce rightRESwitch = Bounce();

// Foot Switches

Bounce leftToggleFS = Bounce();
Bounce rightToggleFS = Bounce();
Bounce leftMomentaryFS = Bounce();
Bounce rightMomentaryFS = Bounce();

// Analog Reads on the Pots

ResponsiveAnalogRead leftTopPot(A13,false);
ResponsiveAnalogRead leftBottomPot(A12,false);
ResponsiveAnalogRead middleTopPot(A11,false);
ResponsiveAnalogRead middleBottomPot(A10,false);
ResponsiveAnalogRead rightTopPot(A8,false);
ResponsiveAnalogRead rightBottomPot(A2,false);
ResponsiveAnalogRead sixWay(A0,false);

// Rotary Encoders
// Encoder name(DT, CLK)
Encoder leftRE(28,31);
Encoder middleRE(32,30);
Encoder rightRE(5,3);

unsigned long previousREUpdate = 0;
unsigned long REMillisInterval = 40;

long leftREPos = 0;
long middleREPos = 0;
long rightREPos = 0;


// audio objects:

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


// ILI9341 attached to SPI2
// CS tied to ground, RST tied to 3v3
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
bool displayNeedsUpdate = false;

// main menus ... should these be pointers?
uint8_t currentMenu = 0;
uint8_t lastMenuSelection = 0;
// menu up/down selections
//uint8_t currentMenuSelection = 0;
uint8_t mainMenuPosition = 0;
uint8_t modMenuPosition = 0;
uint8_t assignmentMenuPosition = 0;

/*
 * 
 * menu stuff
 *
 */

char* mainMenu[] = {
  "Stuff and things!",
  "foo & bar",
  "baz & qux",
  "chumblespuzz + xj",
  "another one",
  "do it again",
  "Captain",
  "Jenkins"
};

uint8_t mainMenuSize = 8;

// global settings
// this might have to be built dynamically.... guuh.
char* modMenu[] = {
  "Master Volume",
  "Master Tone",
  "Care,",
  "Accuracy",
  "Spelllchekin",
  "Illicit Intake",
  "Self Loathing",
  "Tits"
};

uint8_t modMenuSize = 8;

// moar stuff !!! D:

char* menuTitles[] = {
  "Sounds",
  "Mods",
  "Assign"
};
uint8_t menuTitlesLength = 3;

//char* activeMenu = menuTitles[0];
// this  might not be needed?
void adjustMenuPointers(int8_t) {
  // stuff
  // point title pointer to array element
  // point 
}

// TODO uuuuuuuuuhhhgh
// array of pointers to pointers of pointers?




/*
 * 
 * Functions below! setup and etcetera
 * 
 */


/*
 * tft print vars
 */

char* fontSize1Line = "-----------------------------------------------------";
char* fontSize2Line = "--------------------------";
char* fontSize3Line = "-----------------";
char* fontSize4Line = "-------------";
char* fontSize5Line = "----------";
char* fontSize6Line = "--------";


// aside from vanity changes, this should be okay
void welcomeScreen() {
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED);  
  tft.setTextSize(4);
  tft.println();
  tft.println("     The");
  tft.println(" Audio Hacker");
  tft.setTextSize(1);
  tft.println();
  tft.println(fontSize1Line);
  tft.println();
  tft.setTextColor(ILI9341_YELLOW); 
  tft.setTextSize(2);
  tft.println(" A pedal full of nonesense");
  tft.setTextSize(1);
  tft.println();
  tft.println(fontSize1Line);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);    
  tft.setTextSize(2);
  tft.println("   by William Hofferbert");
  tft.setTextSize(1);
  tft.println();
  tft.println(fontSize1Line);
  tft.println();
  Serial.print("welcomeScreen microseconds: ");
  Serial.println(micros() - start);
}

// this will be a landing point for many things probably
void menuHeader(uint32_t headerColor) {
  tft.setTextColor(headerColor); 
  tft.setCursor(0, 0);
  tft.setTextSize(5);
  // TODO some array of pointers for menu headers
  // should say "Sounds"
  //tft.println(menuTitles[currentMenu]);
  tft.println(menuTitles[lastMenuSelection]);
}

// can i do this with just pointers?
void drawPedalMenu(uint32_t text, uint32_t highlight, uint32_t header) {
  unsigned long start = micros();
  //menuHeader(ILI9341_BLUE);
  menuHeader(header);
  tft.setTextSize(3);
  tft.setTextColor(text);
  for (int i=0; i < mainMenuSize; i++) {
    if (i == mainMenuPosition) {
      //tft.setTextColor(ILI9341_WHITE);
      tft.setTextColor(highlight);
      tft.println(mainMenu[i]);
      tft.setTextColor(text);    
    } else {
      tft.println(mainMenu[i]);
    }
  }
  Serial.print("pedalMenu microseconds: ");
  Serial.println(micros() - start);
}


// handler for screen changes
void updatePedalScreen() {
  // should check if current menu is different than last
  // if so, blank and reset some things
  if (currentMenu != lastMenuSelection) {
    // redraw menu
    if (lastMenuSelection == 0) {
      drawPedalMenu(ILI9341_BLACK,ILI9341_BLACK,ILI9341_BLACK);
      //black text print
    } else if (lastMenuSelection == 1) {
      //black text print
    } else if (lastMenuSelection == 2){
      //black text print
    }
    lastMenuSelection = currentMenu;
  }
  if (currentMenu == 0) {
    drawPedalMenu(ILI9341_GREEN,ILI9341_RED,ILI9341_BLUE);
  } else if (currentMenu == 1) {
    drawPedalMenu(ILI9341_GREEN,ILI9341_RED,ILI9341_BLUE);
  } else if (currentMenu == 2) {
    // something
    return;
  }
  displayNeedsUpdate = false;
}


// left rotary encoder handles main menu up/down stuff
void updateLeftRotaryEncoder() {
  //long newLeft;
  bool updated = false;
  long newLeft = leftRE.read();
  // TODO when mainMenuPosition gets reset, also reset the rotary encoder position
  long diff = newLeft - leftREPos;
  if (diff > 2) {
    mainMenuPosition+=1;
    if (mainMenuPosition >= mainMenuSize) {
      mainMenuPosition = 0;
    }
    updated = true; 
  } else if (diff < -2) {
    if (mainMenuPosition > 0) {
      mainMenuPosition-=1;
    } else {
      mainMenuPosition = mainMenuSize - 1;
    }
    updated = true; 
  }
  if (updated == true) {
    leftREPos = newLeft;
    displayNeedsUpdate = true;
    Serial.print("Left RE new position: ");
    Serial.println(newLeft);  
  }
}

// middle RE handles middle menu (mods/available)
void updateMiddleRotaryEncoder() {
  long newMiddle;
  newMiddle = middleRE.read();
  long diff = newMiddle - middleREPos;
  bool updated = false;
  if (diff > 2) {
    // clockwise
    // main menu right
    updated = true;
  } else if (diff < -2) {
    // ccw
    // main menu left
    updated = true;
  }
  if (updated == true) {
    middleREPos = newMiddle;
    Serial.print("Middle RE new position: ");
    Serial.println(newMiddle);
    // other things,
    
  }
}


// right knob controls mod value assignment tasks 
void updateRightRotaryEncoder() {
  long newRight;
  newRight = rightRE.read();
  if (newRight != rightREPos) {
    rightREPos = newRight;
    Serial.print("Right RE new position: ");
    Serial.println(newRight);
  }
}



// update all the rotary encoder triggers
void updateRotaryEncoders() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousREUpdate > REMillisInterval) {
    previousREUpdate = currentMillis;
    updateLeftRotaryEncoder();
    updateMiddleRotaryEncoder();
    updateRightRotaryEncoder();
  }
}



// not used any more, automatic menu scrolling
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


// loop tasks for frequency stuff
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


void updateREButtons() {
  // stuff
  leftRESwitch.update();
  if (leftRESwitch.fell()) {
    // button pressed
    Serial.println("Pressed left RE button");
    // change menu to modMenu
    if (currentMenu == 0) {
      // update screen to mod menu
      currentMenu = 1;
      displayNeedsUpdate = true;
    }
  }
  middleRESwitch.update();
  rightRESwitch.update();
}

void updateAnalogs() {
  // stuff
  leftTopPot.update();
  leftBottomPot.update();
  middleTopPot.update();
  middleBottomPot.update();
  rightTopPot.update();
  rightBottomPot.update();
  sixWay.update();
  return;
}


void updateFootSwitches() {
  // stuff
  return;
}


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
  leftRESwitch.attach(LEFT_RE_SWITCH_PIN, INPUT_PULLUP);
  middleRESwitch.attach(MIDDLE_RE_SWITCH_PIN, INPUT_PULLUP);
  rightRESwitch.attach(RIGHT_RE_SWITCH_PIN, INPUT_PULLUP);
  leftRESwitch.interval(25);
  middleRESwitch.interval(25);
  rightRESwitch.interval(25);

  // music setup
  notefreq1.begin(.15);

  delay(4200);
  //delay(3250);
  tft.fillScreen(ILI9341_BLACK);
  drawPedalMenu(ILI9341_GREEN, ILI9341_RED, ILI9341_BLUE);
}


// for best effect with freqAndNote, make your terminal/monitor 
// a minimum of 62 chars wide, and as tall as you can.
void loop() {
  // update pots and stuff first
  //displayIntervalTest();
  updateRotaryEncoders();
  updateREButtons();
  updateAnalogs();
  updateFootSwitches();

  // audio interaction
  freqAndNote();
  
  //update screen if we need to
  if (displayNeedsUpdate == true) {
    //Serial.println("updating display ...");
    updatePedalScreen();
  }

}

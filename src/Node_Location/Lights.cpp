#include "Lights.h"
#include "Location.h" // for Index

// LED count in the corners
#define NUM_CORNER 8

const byte PIN_DATA_DEBUG = 24; // A0=D24 
const byte PIN_CLK_DEBUG = 25; // A2=D25
#define NUM_DEBUG 3

CRGB corner[N_RANGE][NUM_CORNER];
CRGB debug[NUM_DEBUG];

#define MASTER_BRIGHTNESS 255

void Lights::begin(Distances *D) {
  Serial << F("Lights. startup.") << endl;

  // corner lights
  // PIN_DATA defined in Location.h
  FastLED.addLeds<WS2811, PIN_DATA0, RGB>(corner[0], NUM_CORNER).setCorrection(TypicalSMD5050);
  FastLED.addLeds<WS2811, PIN_DATA1, RGB>(corner[1], NUM_CORNER).setCorrection(TypicalSMD5050);
  FastLED.addLeds<WS2811, PIN_DATA2, RGB>(corner[2], NUM_CORNER).setCorrection(TypicalSMD5050);
  // debug lights
  FastLED.addLeds<APA102, PIN_DATA_DEBUG, PIN_CLK_DEBUG, RGB>(debug, NUM_DEBUG).setCorrection(TypicalSMD5050);
  
  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

  this->d = D;

  Serial << F("Lights. startup complete.") << endl;
}

//extern const byte Index[N_RANGE]; // defined in Location.h
void Lights::update(systemState state) {

  static CHSV normalColor(HUE_RED, 255, 255);
  normalColor.hue++;

  const CHSV programColor(HUE_BLUE, 255, 255);

  // set color from system state
  CHSV color;
  if( state == M_PROGRAM ) color = programColor;
  else color = normalColor;

  // for each distance reading
  for( byte i=0; i<N_RANGE; i++ ) {
    // assign value from distance
//    byte distance = map(d->D[i], 0, HL, 0, 255-12);
    // unwind the indexing
    byte distance = map(d->D[i], 0, HL, 0, 255-12);

    // assign corner
    for( byte j=0; j<NUM_CORNER; j++ ) {
      corner[i][j] = color;
      corner[i][j].fadeLightBy( distance );
    }
    // assign debug
    debug[i] = color;
    debug[i].fadeLightBy( distance );

  }

  // push
  FastLED.show();
}


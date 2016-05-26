#include "Animations.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_STRIPS][NUM_LEDS];

// startup
void Animation::begin() {
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds[0], NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds[1], NUM_LEDS).setCorrection(TypicalSMD5050);

  this->setFPS();
  this->setMasterBrightness();

  this->startAnimation();

  this->startHue();
  this->incrementHue();  

  this->startPosition();
  this->incrementPosition();

  this->startSeed();

  this->setActivity();

  Serial << F("Animation. Startup complete.") << endl;
}
// sets FPS
void Animation::setFPS(uint16_t framesPerSecond) {
  this->pushNextFrame.interval(1000UL/framesPerSecond);
  Serial << F("FPS= ") << framesPerSecond << F(". show update=") << 1000UL/framesPerSecond << F(" ms.") << endl;
}

// sets master brightness
void Animation::setMasterBrightness(byte masterBrightness) {
  // set master brightness control
  FastLED.setBrightness(masterBrightness); 
  Serial << F("Master brightness= ") << masterBrightness << endl;
}

// sets the animation 
void Animation::startAnimation(byte animation, boolean clearStrip) {
  this->anim = animation % N_ANIMATIONS;
  if( clearStrip ) FastLED.clear();

  Serial << F("animation=") << this->anim << endl;

}
void Animation::startHue(byte hue) {
  this->hueVal = hue;
  Serial << F("hue start=") << this->hueVal << endl;
}
void Animation::incrementHue(int inc) {
  this->hueInc = inc;
  Serial << F("hue increment=") << this->hueInc << endl;
}
void Animation::startPosition(byte pos) {
  this->posVal = pos % NUM_LEDS;
  Serial << F("pos=") << this->posVal << endl;
}
void Animation::incrementPosition(int inc) {
  this->posInc = inc ;
  Serial << F("increment=") << this->posInc << endl;
}
void Animation::startSeed(uint16_t seed) {
  random16_set_seed( seed );  // FastLED/lib8tion
  Serial << F("random seed=") << seed << endl;
}
void Animation::setActivity(fract8 chance) {
  if( chanceAct != chance ) {
    this->chanceAct = chance;
    Serial << F("activity chance=") << this->chanceAct << endl;
  }
}

// runs the animation
void Animation::runAnimation() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if( ! nextFrameReady ) {
    switch( anim ) {
    case A_SOLID: 
      aSolid(); 
      break;
    case A_RAINBOW: 
      aRainbow(); 
      break;
    case A_GLITTER: 
      aGlitter(); 
      break;
    case A_CONFETTI: 
      aConfetti(); 
      break;
    case A_CYLON: 
      aCylon(); 
      break;
    case A_BPM: 
      aBPM(); 
      break;
    case A_JUGGLE: 
      aJuggle(); 
      break;
    case A_WHITE: 
      aWhite(); 
      break;
    case A_FIRE: 
      aFire(); 
      break;
    case A_TESTPATTERN: 
      aTestPattern(); 
      break;
    default: 
      break;
    }
    nextFrameReady = true;
  }    

  // ready to push next frame?  
  if( pushNextFrame.check() ) {
    FastLED.show(); // push

    nextFrameReady = false; // setup for next frame calculation
    pushNextFrame.reset();  // setup for next frame push
  } 

}

// uses: hueVal, hueInc
void Animation::aSolid() {
  // FastLED's built-in solid fill (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_solid( leds[s], NUM_LEDS, CHSV(hueVal, 255, 255) );
  }
  hueVal += hueInc;
}

// uses: hueVal, hueInc
void Animation::aRainbow() {
  // FastLED's built-in rainbow generator (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_rainbow( leds[s], NUM_LEDS, hueVal );
  }
  hueVal += hueInc;
}

// uses: hueVal, hueInc, posVal, chanceAct
void Animation::aGlitter() {
  if( random8() < chanceAct ) {
    posVal += random8(NUM_LEDS);
    posVal %= NUM_LEDS;
    leds[0][posVal] = CHSV( hueVal, 255, 255 );
    leds[1][posVal] = leds[0][posVal];
  }

  for(int s = 0; s < NUM_STRIPS; s++) {
    blur1d( leds[s], NUM_LEDS, 64 );
  }

  hueVal += hueInc;
}

// uses: hueVal, hueInc
void Animation::aConfetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds[0], NUM_LEDS, 10 );
  fadeToBlackBy( leds[1], NUM_LEDS, 10 );

  hueVal += hueInc;
  posVal = random8(NUM_LEDS);

  leds[0][posVal] += CHSV( hueVal, 200, 255 );
  leds[1][posVal] = leds[0][posVal];
}

// uses: hueVal, hueInc, posInc
void Animation::aCylon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds[0], NUM_LEDS, abs(2*posInc) );
  fadeToBlackBy( leds[1], NUM_LEDS, abs(2*posInc) );

  hueVal += hueInc;
  //  posVal = beatsin8(13, 0, NUM_LEDS); // see: lib8tion.h
  posVal = beatsin8(abs(posInc), 0, NUM_LEDS); // see: lib8tion.h

  leds[0][posVal] = CHSV( hueVal, 255, 255 );
  leds[1][posVal] = leds[0][posVal];
}

// uses: hueVal, hueInc
void Animation::aBPM() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  const uint8_t BeatsPerMinute = 120;
  const CRGBPalette16 palette = PartyColors_p;

  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);

  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[0][i] = ColorFromPalette(palette, hueVal+(i*2), beat-hueVal+(i*10));
    leds[1][i] = leds[0][i];
  }

  hueVal += hueInc;
}

// uses: hueVal
void Animation::aJuggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds[0], NUM_LEDS, 20);
  fadeToBlackBy( leds[1], NUM_LEDS, 20);

  //  hueVal = 0;

  for( int i = 0; i < 8; i++) {
    int l = beatsin16(i+7,0,NUM_LEDS);
    leds[0][l] |= CHSV(hueVal, 200, 255);
    leds[1][l] = leds[0][l];
    hueVal += 32;
  }
}

// uses: hueVal (as brightness), hueInc
void Animation::aWhite() {
  // FastLED's built-in solid fill (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_solid( leds[s], NUM_LEDS, CHSV(0, 0, hueVal) );
  }
  hueVal += hueInc;
}

// uses: hueVal, hueInc
void Animation::aFire() {
  
  const CRGBPalette16 gPal = HeatColors_p;
  const bool gReverseDirection = false;
  
  // COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55
//#define COOLING  20

// 54 W with a lot of fire: 10A
// 36 W with these settings: 7.2A

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
//#define SPARKING 50
//#define SPARKING 200

  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < chanceAct ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (NUM_LEDS-1) - j;
    } 
    else {
      pixelnumber = j;
    }
    leds[0][pixelnumber] = color;
    leds[1][pixelnumber] = color;
  }

}

// uses: posVal, posInc, hueVal
void Animation::aTestPattern() {
  // zap the strips
  FastLED.clear();

  // cycle through the LED actuals.
  switch( hueVal ) {
    case HUE_RED: hueVal = HUE_GREEN; break;
    case HUE_GREEN: hueVal = HUE_BLUE; break;
    default: hueVal = HUE_RED; break;
  }
  
  // just look at start of strip and end of strip.
  switch( posVal ) {
    case 0: posVal = NUM_LEDS-1; break;
    default: posVal = 0; break;
  }
  
  leds[0][posVal] = CHSV( hueVal, 255, 255 );
  leds[1][posVal] = leds[0][posVal];
}

Animation A;




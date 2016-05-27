#include "Animations.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

// startup
void Animation::begin() {
  // tell FastLED about the LED strip configuration; mirrored strips
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  random16_add_entropy(analogRead(A0));

  this->setFPS();
  this->setMasterBrightness();

  this->startAnimation();

  Serial << F("Animation. Startup complete.") << endl;
}

// sets FPS
void Animation::setFPS(uint16_t framesPerSecond) {
  this->pushNextFrame.interval(1000UL / framesPerSecond);
  Serial << F("FPS= ") << framesPerSecond << F(". show update=") << 1000UL / framesPerSecond << F(" ms.") << endl;
}

// sets master brightness
void Animation::setMasterBrightness(byte masterBrightness) {
  // set master brightness control
  FastLED.setBrightness(masterBrightness);
  Serial << F("Master brightness= ") << masterBrightness << endl;
}

// sets the animation
void Animation::startAnimation(byte animation, boolean clearStrip) {
  if ( this->anim != animation % N_ANIMATIONS ) {
    this->anim = animation % N_ANIMATIONS;
    if ( clearStrip ) FastLED.clear();

    Serial << F("animation=") << this->anim << endl;
  }
}

// runs the animation
void Animation::runAnimation() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if ( ! nextFrameReady ) {
    switch ( anim ) {
      case A_FIRE:
        aFire();
        break;
      case A_SHADOW:
        aShadow();
        break;
    }
    nextFrameReady = true;
  }

  // ready to push next frame?
  if ( pushNextFrame.check() ) {
    FastLED.show(); // push

    nextFrameReady = false; // setup for next frame calculation
    pushNextFrame.reset();  // setup for next frame push
  }

}

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
#define SPARKING 120
  //#define SPARKING 200

  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS / 2) + 2));
  }

  //  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  //  for( int k= NUM_LEDS - 1; k >= 2; k--) {
  //    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  //  }

  // Step 2.  Heat from each cell drifts left and right and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= NUM_LEDS / 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  for ( int k = 0; k <= NUM_LEDS / 2; k++) {
    heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8((NUM_LEDS / 2) - 2, (NUM_LEDS / 2) + 3);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    }
    else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }

}

void Animation::aShadow() {
  // cool the LEDS
  fadeToBlackBy( leds, NUM_LEDS, 10 );

  leds[this->shadowLoc] = CHSV(HUE_BLUE, 255, 255);

  int extent = this->shadowLoc - this->shadowArea;
  extent = extent < 0 ? 0 : extent;

  leds[extent] = CHSV(HUE_GREEN, 255, 255);

  extent = this->shadowLoc + this->shadowArea;
  extent = extent > NUM_LEDS-1 ? NUM_LEDS-1 : extent;

  leds[extent] = CHSV(HUE_GREEN, 255, 255);
}

void Animation::setShadowCenter(byte loc) {
  this->shadowLoc = loc;
}
void Animation::setShadowExtent(byte area) {
  this->shadowArea = area;
}

Animation A;




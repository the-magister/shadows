#include "Animation.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

// startup
void Animation::begin(byte startPos, byte startIntensity) {
  // tell FastLED about the LED strip configuration; mirrored strips
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  random16_add_entropy(analogRead(A0));

  this->setFPS();
  this->setMasterBrightness();

  this->currentPos = this->targetPos = startPos;
  this->currentIntensity = this->targetIntensity = startIntensity;

  Serial << F("Animation. Startup complete.") << endl;
}

// sets FPS
void Animation::setFPS(uint16_t framesPerSecond) {
  this->fps = framesPerSecond;
  this->pushNextFrame.interval(1000UL / framesPerSecond);
  Serial << F("Animation. FPS= ") << framesPerSecond << F(". show update=") << 1000UL / framesPerSecond << F(" ms.") << endl;
}

// sets master brightness
void Animation::setMasterBrightness(byte masterBrightness) {
  // set master brightness control
  FastLED.setBrightness(masterBrightness);
  Serial << F("Animation. Master brightness= ") << masterBrightness << endl;
}

// runs the animation
void Animation::update() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if ( ! nextFrameReady ) {

    // do we need to shift the center of the animation?
    if ( this->currentPos > this->targetPos ) {
      this->currentPos--;
    } else if ( this->currentPos < this->targetPos ) {
      this->currentPos++;
    }

    // do we need to shift the intensity of the animation?
    if ( this->currentIntensity > this->targetIntensity ) {
      this->currentIntensity--;
    } else if ( this->currentIntensity < this->targetIntensity ) {
      this->currentIntensity++;
    }
    
    this->fireAnimation();

    nextFrameReady = true;
  }

  // ready to push next frame?
  if ( pushNextFrame.check() ) {
    FastLED.show(); // push

    nextFrameReady = false; // setup for next frame calculation
    pushNextFrame.reset();  // setup for next frame push
  }

}
void Animation::fadePositionTo(byte pixel) {
  if ( this->targetPos != pixel ) {
    Serial << F("Animation. fadePositionTo= ") << pixel << endl;
    this->targetPos = pixel;
  }
}

void Animation::fadeIntensityTo(byte intensity) {
  if ( this->targetIntensity != intensity ) {
    Serial << F("Animation. fadeIntensityTo= ") << intensity << endl;
    this->targetIntensity = intensity;
  }
}

void Animation::fireAnimation() {

  const CRGBPalette16 gPal = HeatColors_p;
  const bool gReverseDirection = false;

  byte leftPos = constrain(this->currentPos, 0, NUM_LEDS - 1);
  byte rightPos = constrain(this->currentPos + 1, 0, NUM_LEDS - 1);

  byte cooling = map(this->currentIntensity, 0, 255, 30, 10);
 
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  // Less cooling = taller flames.  More cooling = shorter flames.
  for ( int k = 0; k < NUM_LEDS; k++) {
    heat[k] = qsub8( heat[k],  random8(0, cooling));
  }

  // Step 2.  Heat from each cell drifts left and right and diffuses a little
  for ( int k = 0; k <= (int)leftPos - 2; k++) {
    heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2] ) / 3;
  }
  for ( int k = NUM_LEDS - 1; k >= (int)rightPos - 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near center position
  if ( random8() < 128 ) {
//    heat[leftPos] = qadd8( heat[leftPos], random8(map(this->currentIntensity,0,255,64,255), 255) );
    heat[leftPos] = qadd8( heat[leftPos], map(this->currentIntensity,0,255,128,255) );
//    heat[rightPos] = qadd8( heat[rightPos], random8(map(this->currentIntensity,0,255,64,255), 255)  );
    heat[rightPos] = qadd8( heat[rightPos], map(this->currentIntensity,0,255,128,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    CRGB color = ColorFromPalette( gPal, scale8( heat[j], 240));
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    }
    else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }

/*
  static Metro dumpInterval(5000UL);
  if( dumpInterval.check() ) {
    Serial << F("Dump. leftPos=") << leftPos << F(" rightPos=") << rightPos;
    Serial << F(" cooling=") << cooling;

    Serial << endl;
  }
*/
}

Animation A;




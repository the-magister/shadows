#include "Animation.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

// startup
void Animation::begin(byte startPos, byte startSpark, byte startCooling) {
  // tell FastLED about the LED strip configuration; mirrored strips
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  random16_add_entropy(analogRead(A0));

  this->setFPS();
  this->setMasterBrightness();

  this->currentPos = this->targetPos = startPos;
  this->currentSpark = this->targetSpark = startSpark;
  this->currentCooling = this->targetCooling = startCooling;

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

// runs the animation
void Animation::update() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if ( ! nextFrameReady ) {
    
    // do we need to shift the center of the animation?
    if( this->currentPos > this->targetPos ) {
      this->currentPos--;
    } else if( this->currentPos < this->targetPos ) {
      this->currentPos++;
    }

    // do we need to shift the spark probability of the animation?
    if( this->currentSpark > this->targetSpark ) {
      this->currentSpark--;
    } else if( this->currentSpark < this->targetSpark ) {
      this->currentSpark++;
    }
    
    // do we need to shift the cooling of the animation?
    if( this->currentCooling > this->targetCooling ) {
      this->currentCooling--;
    } else if( this->currentCooling < this->targetCooling ) {
      this->currentCooling++;
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
  this->targetPos = pixel;
}

void Animation::fadeSparksTo(byte spark) {
  this->targetSpark = spark;
}

void Animation::fireAnimation() {

  const CRGBPalette16 gPal = HeatColors_p;
  const bool gReverseDirection = false;
  
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  // Less cooling = taller flames.  More cooling = shorter flames.
  // Default 55, suggested range 20-100
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, (((int)this->currentCooling * 10) / NUM_LEDS / 2) + 2));
  }

  //  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  //  for( int k= NUM_LEDS - 1; k >= 2; k--) {
  //    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  //  }

  // Step 2.  Heat from each cell drifts left and right and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= this->currentPos; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
  for ( int k = 0; k <= this->currentPos; k++) {
    heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near center position
  if ( random8() < this->currentSpark ) {
    byte y = constrain((int)this->currentPos+(int)(random8(0,3)-1), 0, NUM_LEDS-1);
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

Animation A;




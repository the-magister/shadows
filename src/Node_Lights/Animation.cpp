#include "Animation.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

// startup
void Animation::begin(byte startPos, byte startIntensity, byte startAnim) {
  // tell FastLED about the LED strip configuration; mirrored strips
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  random16_add_entropy(analogRead(A0));

  this->setFPS();
  this->setMasterBrightness();
  this->position = startPos;
  this->intensity = startIntensity;

  this->setAnimation(startAnim);

  Serial << F("Animation. Startup complete.") << endl;
}

// sets FPS
void Animation::setFPS(uint16_t framesPerSecond) {
  this->fps = framesPerSecond;
  this->pushInterval = 1000UL/framesPerSecond - 5; // fudge a little faster
  this->pushNextFrame.interval(this->pushInterval);
  Serial << F("Animation. FPS= ") << framesPerSecond << F(". show update=") << this->pushInterval << F(" ms.") << endl;
}

// sets master brightness
void Animation::setMasterBrightness(byte masterBrightness) {
  // set master brightness control
  FastLED.setBrightness(masterBrightness);
  Serial << F("Animation. Master brightness= ") << masterBrightness << endl;
}

void Animation::setAnimation(byte animation, boolean clearStrip) {

  this->anim = animation;

  if ( clearStrip ) fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
}

void Animation::setPosition(byte position) {
  if ( this->position != position ) {
    Serial << F("Animation. position= ") << position << endl;
    this->position = position;
  }
}

void Animation::setIntensity(byte intensity) {
  if ( this->intensity != intensity ) {
    Serial << F("Animation. intensity= ") << intensity << endl;
    this->intensity = intensity;
  }
}

// runs the animation
void Animation::update() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if ( ! nextFrameReady ) {

    /*
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
    */
    
    switch ( anim ) {
      case A_IDLE:
        aCylon( this->intensity );
        break;
      case A_OUTPLANE:
        aCylon( this->intensity );
        break;
      case A_INPLANE:
        aProjection( this->position, map(this->intensity, 0, 255, NUM_LEDS/2, 10) );
         break;
    }
    
    nextFrameReady = true;
  }

  // ready to push next frame?
  if ( pushNextFrame.check() ) {
    FastLED.show(); // push

    static byte pushCount=0;
    pushCount++;

    if( pushCount==100 ) {
      pushCount = 0;
   
      int repFPS = FastLED.getFPS();
  
      if( repFPS > 0 && repFPS > this->fps+1 ) {
        this->pushInterval++;
        this->pushNextFrame.interval(this->pushInterval);
        Serial << F("Animation. fps reported=") << repFPS << F(" pushInterval=") << this->pushInterval << endl;
      }
      if( repFPS>0 && repFPS < this->fps-1 ) {
        this->pushInterval--;
        this->pushInterval = this->pushInterval < 0 ? 0 : this->pushInterval;
        this->pushNextFrame.interval(this->pushInterval);
        Serial << F("Animation. fps reported=") << repFPS << F(" pushInterval=") << this->pushInterval << endl;
      }
    }
    
    nextFrameReady = false; // setup for next frame calculation
    pushNextFrame.reset();  // setup for next frame push
    
  }

}

void Animation::aCylon(byte bright) {
  // a colored dot sweeping back and forth, with fading trails

  // fade everything
  fadeToBlackBy( leds, NUM_LEDS, 10 );

  // set the speed the pixel travels 
  byte posVal = beatsin8(13, 0, NUM_LEDS); // see: lib8tion.h

  // cycle through hues, using intensity to set value
  static byte hue = 0;
  leds[posVal] = CHSV(hue++, 255, bright );
}

void Animation::aProjection(byte center, byte extent) {
  // a patch of color

  // fade everything
//  fadeToBlackBy( leds, NUM_LEDS, 10 );

  // clear everything
  fill_solid( leds, NUM_LEDS, CRGB(0,0,0) );
  
  // cycle through hues
  static byte hue = 0;
  hue++;

  // how far are we diffusing?
  byte right_s = constrain((int)qadd8(center, 1), 0, NUM_LEDS-1);
  byte right_e = constrain((int)qadd8(right_s, extent), 0, NUM_LEDS-1);
  byte left_s = constrain((int)qsub8(center, 1), 0, NUM_LEDS-1);
  byte left_e = constrain((int)qsub8(left_s, extent), 0, NUM_LEDS-1);

  byte brightness = map(constrain(2*extent, 0, NUM_LEDS-1), 0, NUM_LEDS-1, 255, 16); 
  for( byte i=left_e;i<=right_e;i++) {
    leds[i] = CHSV(hue, 255, brightness);
//    leds[i] = CHSV(hue, 255, map(triwave8(map(i,left_e,right_e,0,255)), 0, 255, 0, bright_max) );
  }

  // and show the extents
  leds[left_e] = CHSV(0, 0, 32);
  leds[left_s] = CHSV(0, 0, 128);
  leds[center] = CHSV(0, 0, 255);
  leds[right_s] = CHSV(0, 0, 128);
  leds[right_e] = CHSV(0, 0, 32);

  // blur everything
//  for( byte i=0; i<extent; i ++ )
//    blur1d( leds, NUM_LEDS, 128 );
 
}

/*
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

  static Metro dumpInterval(5000UL);
  if( dumpInterval.check() ) {
    Serial << F("Dump. leftPos=") << leftPos << F(" rightPos=") << rightPos;
    Serial << F(" cooling=") << cooling;

    Serial << endl;
  }
}
*/

Animation A;




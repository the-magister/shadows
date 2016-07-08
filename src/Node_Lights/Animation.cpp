#include "Animation.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

// startup
void Animation::begin() {
  // tell FastLED about the LED strip configuration; mirrored strips
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  random16_add_entropy(analogRead(A0));

  this->setMasterBrightness(255);
  this->position = this->currentPosition = NUM_LEDS/2;
  this->extent = this->currentExtent = 0;

  this->setAnimation(A_IDLE);

  Serial << F("Animation. Startup complete.") << endl;
}

/*
// sets FPS
void Animation::setFPS(uint16_t framesPerSecond) {
  this->fps = framesPerSecond;
  this->pushInterval = 1000UL/framesPerSecond - 5; // fudge a little faster
  this->pushNextFrame.interval(this->pushInterval);
  Serial << F("Animation. FPS= ") << framesPerSecond << F(". show update=") << this->pushInterval << F(" ms.") << endl;
}
*/

// sets master brightness
void Animation::setMasterBrightness(byte masterBrightness) {
  // set master brightness control
  FastLED.setBrightness(masterBrightness);

  static byte lastBright = 0;
  if( lastBright != masterBrightness ) {
    Serial << F("Animation. Master brightness= ") << masterBrightness << endl;
    lastBright = masterBrightness;
  }
}

void Animation::setAnimation(byte animation, boolean clearStrip) {

  this->anim = animation;

  if ( clearStrip ) fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
}

void Animation::setCenter(byte position) {
  if ( this->position != position ) {
    Serial << F("Animation. position= ") << position << endl;
    this->position = position;
  }
}

void Animation::setExtent(byte extent) {
  if ( this->extent != extent ) {
    Serial << F("Animation. extent= ") << extent << endl;
    this->extent = extent;
  }
}

// runs the animation
void Animation::update() {
 
    // do we need to shift the center of the animation?
    if ( this->currentPosition > this->position ) {
      this->currentPosition -= (this->currentPosition - this->position)/2 +1;
    } else if ( this->currentPosition < this->position ) {
      this->currentPosition += (this->position - this->currentPosition)/2 +1;
    }

    // do we need to shift the extent of the animation?
    if ( this->currentExtent > this->extent ) {
      this->currentExtent -= (this->currentExtent - this->extent)/2 +1;
    } else if ( this->currentExtent < this->extent ) {
      this->currentExtent += (this->extent - this->currentExtent)/2 +1;
    }
    
    switch ( anim ) {
      case A_IDLE:
        aCylon( 255 );
        break;
      case A_INPLANE:
        aProjection( this->currentPosition, this->currentExtent );
         break;
      case A_CALIBRATE:
        aSolid( CRGB(0,255,0) );
         break;
      case A_PROGRAM:
        aSolid( CRGB(0,0,255) );
         break;
    }
/*    
    nextFrameReady = true;
  }
*/
  // ready to push next frame?
//  if ( pushNextFrame.check() ) {
    FastLED.show(); // push

    static word pushCount=0;
    pushCount++;

    if( pushCount==1000 ) {
      pushCount = 0;
   
      int repFPS = FastLED.getFPS();
  
  //    if( repFPS > 0 && repFPS > this->fps+1 ) {
  //      this->pushInterval++;
  //      this->pushNextFrame.interval(this->pushInterval);
  //      Serial << F("Animation. fps reported=") << repFPS << F(" pushInterval=") << this->pushInterval << endl;
        Serial << F("Animation. fps reported=") << repFPS  << endl;
/*      }
      if( repFPS>0 && repFPS < this->fps-1 ) {
        this->pushInterval--;
        this->pushInterval = this->pushInterval < 0 ? 0 : this->pushInterval;
        this->pushNextFrame.interval(this->pushInterval);
        Serial << F("Animation. fps reported=") << repFPS << F(" pushInterval=") << this->pushInterval << endl;
      }
*/      
    }
    
//    nextFrameReady = false; // setup for next frame calculation
//    pushNextFrame.reset();  // setup for next frame push
    
//  }

}

void Animation::aCylon(byte bright) {
  // a colored dot sweeping back and forth, with fading trails

  // two different bpm; one for CW, one for CCW.
  const byte bpm[]={16,8};
  static unsigned long tbase = millis();
  static byte bpmI = 0;
  const word phase[] = {(65535/4)*3, 65535/4}; // 3/4*PI phase change, so we start at pixel 0
  
  // fade everything
  fadeToBlackBy( leds, NUM_LEDS, bpm[bpmI] );

  // set the speed the pixel travels 
  byte posVal = map(
    beatsin16(bpm[bpmI], 0, 65535, tbase, phase[bpmI]),
    0, 65535, 0, NUM_LEDS
  ); // see: lib8tion.h

  // change bpm?
  if ( posVal==NUM_LEDS-1 && bpmI==0 && millis() > tbase+500UL ) { 
      bpmI = 1;
      tbase = millis();
  }
  if ( posVal==0 && bpmI==1 && millis() > tbase+500UL ) { 
      bpmI = 0;
      tbase = millis();
  }
  
  // cycle through hues, using extent to set value
  static byte hue = 0;
  leds[posVal] = CHSV(hue++, 255, bright );
}

void Animation::aProjection(byte center, byte extent) {
  // https://github.com/FastLED/FastLED/wiki/Pixel-reference
  
  // what are the extents
  byte more_s = constrain((int)qadd8(center, 1), 0, NUM_LEDS-1);
  byte more_e = constrain((int)qadd8(more_s, extent), 0, NUM_LEDS-1);
  byte less_s = constrain((int)qsub8(center, 1), 0, NUM_LEDS-1);
  byte less_e = constrain((int)qsub8(less_s, extent), 0, NUM_LEDS-1);

  // fade everything
  fadeToBlackBy( leds, NUM_LEDS, 10 );

  // clear everything
//  fill_solid( leds, NUM_LEDS, CRGB(0,0,0) );

  // shift the animation away from the center point
//  for( byte i=0; i<center; i++ ) { leds[i]=leds[i+1]; }
//  for( byte i=NUM_LEDS-1; i>center; i-- ) { leds[i]=leds[i-1]; }
//  memmove8( &leds[more_s], &leds[center], (NUM_LEDS-more_s-1) * sizeof( CRGB) );
//  memmove8( &leds[less_s], &leds[center], (less_s-0-1) * sizeof( CRGB) );
  
  // cycle through hues
  static byte hue = 0;
  hue++;

  // set brightness
  byte brightness = scale8_video(
    map(constrain(extent, 0, NUM_LEDS/2), 0, NUM_LEDS/2, 255, 0), 
    255
  );
  this->setMasterBrightness(brightness);
  
  CHSV color_start = CHSV(hue, 255, 255);
  CHSV color_end = CHSV(hue+128, 255, 255);

/*
  byte brightness = map(constrain(extent, 0, NUM_LEDS/2), 0, NUM_LEDS/2, 255, 16); 
  for( byte i=left_e;i<=right_e;i++) {
    leds[i] = CHSV(hue, 255, brightness);
  }
*/
  // start from right_s and got to right_e
  fill_gradient( leds, less_s, color_start, less_e, color_end );
  // start from left_s and got to right_e
  fill_gradient( leds, more_s, color_start, more_e, color_end );

  // blur everything
//  for( byte i=0; i<extent; i ++ )
//    blur1d( leds, NUM_LEDS, 128 );

  // show the extents
  leds[less_e] = CHSV(hue, 255, 32);
  leds[less_s] = CHSV(hue, 255, 128);
  leds[center] = CHSV(hue, 255, 255);
  leds[more_s] = CHSV(hue, 255, 128);
  leds[more_e] = CHSV(hue, 255, 32);

 
}


void Animation::aSolid(CRGB color) {
  fadeToBlackBy( leds, NUM_LEDS, 10 );
  fill_solid(leds, NUM_LEDS, color);
}

/*
void Animation::fireAnimation() {

  const CRGBPalette16 gPal = HeatColors_p;
  const bool gReverseDirection = false;

  byte leftPos = constrain(this->currentPos, 0, NUM_LEDS - 1);
  byte rightPos = constrain(this->currentPos + 1, 0, NUM_LEDS - 1);

  byte cooling = map(this->currentExtent, 0, 255, 30, 10);

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
//    heat[leftPos] = qadd8( heat[leftPos], random8(map(this->currentExtent,0,255,64,255), 255) );
    heat[leftPos] = qadd8( heat[leftPos], map(this->currentExtent,0,255,128,255) );
//    heat[rightPos] = qadd8( heat[rightPos], random8(map(this->currentExtent,0,255,64,255), 255)  );
    heat[rightPos] = qadd8( heat[rightPos], map(this->currentExtent,0,255,128,255) );
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




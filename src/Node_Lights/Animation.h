#ifndef Animation_h
#define Animation_h

#include <Arduino.h>

#include <Streaming.h>

#define PIN_DATA1      4
#define PIN_DATA2      7
#define PIN_CLK        6
#define LED_TYPE       APA102
#define NUM_STRIPS     2
#define NUM_LEDS       110

#include <Metro.h>
#include <FastLED.h>

// at 60mA per pixel on white * 220 pixels, 13.2A @ 5VDC.
// by ammeter, I get:
// 6A @ 5VDC (White)
// 3.4A (rainbow)
// 1A sparse animations

// 4A supply seems about right

FASTLED_USING_NAMESPACE

// enumerate animation modes
enum animation_t {
  A_IDLE=0,
  A_INPLANE,
  A_CALIBRATE,
  A_PROGRAM,
  
  N_ANIMATIONS
};

class Animation {
  public:
    // initialize led strips
    void begin();
    // which calls the following functions with their defaults:
    
    // set frames per second
    void setFPS(uint16_t framesPerSecond=50);
    // set master brightness
    void setMasterBrightness(byte masterBrightness=255);

    // animation control
    void setAnimation(byte animation=A_IDLE, boolean clearStrip=true);
    void setCenter(byte position);
    void setExtent(byte extent);
    
    // updates the animation
    void update();

  private:
    byte anim;
    
    byte position, extent;
    byte currentPosition, currentExtent;
    
    // animation layer
    void aCylon(byte bright);
    void aCylonSimple(byte bright);
    void aProjection(byte pos, byte extent);
    void aSolid(CRGB color);
    void aFire(byte pos, byte extent);
    void aFireIdle();
};

extern Animation A;

#endif

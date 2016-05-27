#ifndef Animations_h
#define Animations_h

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
  A_FIRE=0, // simulated fire colors, used when no object is present
  A_SHADOW, // inverse shadow of projected object
  
  N_ANIMATIONS
};

class Animation {
  public:
    // initialize led strips
    void begin();
    // which calls the following functions with their defaults:
    // set frames per second
    void setFPS(uint16_t framesPerSecond=30);
    // set master brightness
    void setMasterBrightness(byte masterBrightness=255);

    // animations control
    // sets the animation 
    void startAnimation(byte animation=A_FIRE, boolean clearStrip=true); 

    // runs the animation
    void runAnimation();

    // sets the shadow animation information
    void setShadowCenter(byte loc);
    void setShadowExtent(byte area);
    
  private:
    byte anim;
    
    Metro pushNextFrame;
    
    void aFire();
    void aShadow();

    byte shadowLoc, shadowArea;
};

extern Animation A;

#endif

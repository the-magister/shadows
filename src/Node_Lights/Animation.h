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

class Animation {
  public:
    // initialize led strips
    void begin(byte startPos=NUM_LEDS/2, byte startIntensity=0);
    // which calls the following functions with their defaults:
    
    // set frames per second
    void setFPS(uint16_t framesPerSecond=30);
    // set master brightness
    void setMasterBrightness(byte masterBrightness=255);

    // animation function
    void fireAnimation();
    
    // animation control
    void fadePositionTo(byte pixel);
    void fadeIntensityTo(byte intensity);
    
    // updates the animation
    void update();

  private:
    Metro pushNextFrame;

    byte currentPos, targetPos;
    byte currentIntensity, targetIntensity;

    byte flareProb;
    byte fps;
};

extern Animation A;

#endif

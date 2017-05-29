#ifndef Lights_h
#define Lights_h

#include <Arduino.h>

#include <FastLED.h>
#include <Network.h>
#include <Streaming.h>
#include <Metro.h>

class Lights {
  public:
    // startup
    void begin(Distances *D);

    // update corner and debug lighting
    void update(systemState state);

  private:
    // store a pointer to the distance object
    Distances * d;
};

#endif

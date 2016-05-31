#ifndef Location_h
#define Location_h

#include <Arduino.h>

#include <Streaming.h>

#include <Network.h>

#define RANGE_PIN 6
#define PW_PIN 7

class Location {
  public:
    // initialize location
    void begin(byte myNodeID);

    // read distance
    void readDistance(Message &msg);

    // calculate positions
    void calculatePosition(Message &msg);
    
    void heavyLift(byte leftRange, byte rightRange, byte acrossRange, byte &rInter, byte &rRange);

  private:
    byte rangePin, pwPin;
    byte myIndex;
    boolean calibrated;
    
    float areaFromDistances(float lA, float lB, float lC);
    float yFromArea(float area, float base);
    float xFromHeight(float y, float l);
};

extern Location L;

#endif

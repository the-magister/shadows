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
    void begin();

    // read distance
    byte readDistance();

    // calculate positions
    void calculatePosition(Message msg);
    
  private:
    byte rangePin, pwPin;
    boolean calibrated;
    
    float areaFromDistances(float lA, float lB, float lC);
    float yFromArea(float area, float base);
    float xFromHeight(float y, float l);

};

extern Location L;

#endif

#ifndef Location_h
#define Location_h

#include <Arduino.h>

#include <Streaming.h>

#include <Network.h>

#define POWER_PIN 5 // wire to MaxSonar +5V (pin 6)
#define RANGE_PIN 6 // wire to MaxSonar RX (pin 4)
#define PW_PIN 7    // wire to MaxSonar PW (pin 2)

class Location {
  public:
    // initialize location
    void begin(byte myNodeID);

    // calibrate sensor;  should be done periodically to adjust for changing temperature and humidity
    void calibrateDistance();

    // read distance
    void readDistance(Message &msg);

    // calculate positions
    void calculatePosition(Message &msg);
    
    void heavyLift(word leftRange, word rightRange, word acrossRange, word &rInter, word &rRange);

  private:
    byte myIndex;
    boolean calibrated;
    
    float areaFromDistances(float lA, float lB, float lC);
    float yFromArea(float area, float base);
    float xFromHeight(float y, float l);
};

extern Location L;

#endif

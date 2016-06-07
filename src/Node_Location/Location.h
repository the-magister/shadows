#ifndef Location_h
#define Location_h

#include <Arduino.h>

#include <Streaming.h>

#include <Network.h>

// Moteuino R4 pins already in use:
//  D2, D10-13 - transceiver
//  D9         - LED
//  D8, D11-13 - flash

#define PIN_GND 3   // DC return
#define PIN_VCC 4   // Vcc in range 3-5.5VDC
#define PIN_RX  6   // hold high/open for ranging. hold low to stop ranging.
#define PIN_PW  7   // pulse width representation with scale factor of 140 uS per inch

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
    
//    void heavyLift(word leftRange, word rightRange, word acrossRange, word &rInter, word &rRange);
    void simpleLift(word leftRange, word rightRange, word &rInter, word &rRange);

  private:
    byte myIndex;
    boolean calibrated;

    /*
    float areaFromDistances(float lA, float lB, float lC);
    float yFromArea(float area, float base);
    float xFromHeight(float y, float l);
    */
};

extern Location L;

#endif

#ifndef Location_h
#define Location_h

#include <Arduino.h>

#include <Network.h>
#include <Streaming.h>
#include <Metro.h>

#define PIN_START_RANGE 5 // trigger for ranging start; read the fps; note this is Dn
const byte rangePin[N_RANGE] = { 7, 6, 5 }; // range from sonar 10, 11, 12, respectivel; note this is An

/*
Physical layout:
    ---- SL --->
         20
   11 -------- 12   ^
     \        /     |
      \      /      HL
    22 \    / 21    |
        \  /        |
         10         |

Tens digit:
  1 = nodes with ultrasound rangefinders (Node_Location)
  2 = nodes with RGB LED strips (Node_Light)

Ones digit (with same tens digit):
  +1 (modulo) = "to my right"/next
  -1 (modulo) = "to my left"/previous

*/


class Location {
  public:
    // startup
    void begin(Distances *D);

    // run an update; calls calculateLocation after.
    void update();
    word reading[N_RANGE]; // for debugging purposes.
    
  private:
    byte left(byte i);
    byte right(byte i);

    // store a pointer to the distance object
    Distances * d;

    // given the distances to the object, calculate the location of the object
    void calculateLocation();

    // helper functions
    unsigned long squared(word x);
    word squareRoot(unsigned long x);

    word semiPerimeter(byte i);
    word altitudeHeight(byte i);
    void correctAltitudeHeight();
    word altitudeBase(byte i);
    word collinearBase(byte i);
    word collinearHeight(byte i);
    word area(byte i);

};

#endif

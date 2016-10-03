#ifndef Location_h
#define Location_h

#include <Arduino.h>

#include <Network.h>
#include <Streaming.h>

#define PIN_START_RANGE 5 // trigger for ranging start; read the fps
#define PIN_RANGE_1 7   // range from sonar 10; note this is A7
#define PIN_RANGE_2 5   // range from sonar 11; note this is A5
#define PIN_RANGE_3 6   // range from sonar 12; note this is A6

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

    // run an update; returns true if there's a change in values
    boolean update();

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
    byte altitudeHeight(byte i);
    void correctAltitudeHeight();
    byte altitudeBase(byte i);
    byte collinearBase(byte i);
    byte collinearHeight(byte i);
    byte area(byte i);
};

#endif

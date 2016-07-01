#ifndef Network_h
#define Network_h

#include <Arduino.h>

#include <Streaming.h>

#define N_NODES		3 // 3 Lights, 3 Location nodes

// geometry of the devices, 16-bit
#define SL			755U // sensor-sensor distance; i.e. side length
#define HL			654U // sensor-LED distance; i.e. altitude
#define IN_PLANE	625U // sensor-LED distance threshold for "detected something"
#define LL			720U // LED strip length; 

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
  
Ultrasound ring goes clockwise round-robin:

  Rx From	Is Next		Tx To
  10		11			12
  11		12			10
  12		10			11
  
*/ 

// NOTE: I specifically skipped getter/setter functions to 
// strip the memory usage down to be as small as possible.
// Stylistically, this is ugly, and it does exposure private 
// components to unintended alteration.  However, both the 
// sensor and lighting nodes use this code so I want a very
// small memory footprint. I did use some bytes for indexing
// variables, as I felt that the readibility of the code
// was greatly improved, so this is a compromise position.

class Location {
  public:
	void begin(word *distance);
	// given the distances to the object, calculate the location of the object
	void calculateLocation();
	// which sets the following target information
	word Ab[N_NODES], Ah[N_NODES]; // object location relative to LEDs, altitude basis
	word Cb[N_NODES], Ch[N_NODES]; // object location relative to LEDs, collinear basis
	word Area[N_NODES]; // relative area of the triangle defined by the object and LEDs, relative to total area.

  private:
	byte left(byte i);
	byte right(byte i);
	
	word *distance;
	
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

extern Location L;

#endif

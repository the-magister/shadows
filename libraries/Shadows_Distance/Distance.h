#ifndef Distance_h
#define Distance_h

#include <Arduino.h>

#include <Streaming.h>

// Moteuino R4 pins already in use:
//  D2, D10-13 - transceiver
//  D9         - LED
//  D8, D11-13 - flash

#define PIN_LED_CLK 3		// corner LED clock line
#define PIN_LED_DATA 4		// corner LED data line
#define PIN_START_RANGE 5	// trigger for ranging start
#define PIN_RANGE_1 7		// range from sonar 1; note this is A5
#define PIN_RANGE_2 5		// range from sonar 1; note this is A6
#define PIN_RANGE_3 6		// range from sonar 1; note this is A7

#define N_RANGE 3			// number of ranging elements (sensors)

class Distance {
  public:
    // initialize sensor 
    void begin();

	// run an update; returns true if there's a change in values
	boolean update(); 
	
    // distance; distance in in*10 to the target (decainches)
    word distance[N_RANGE];
	
};

extern Distance D;

#endif

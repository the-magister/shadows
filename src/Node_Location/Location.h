#ifndef Location_h
#define Location_h

#include <Arduino.h>

#include <Network.h>
#include <Streaming.h>
#include <Metro.h>

/*
Physical layout:
    ---- SL --->
 (YEL)   20    (GRN)
   11 -------- 12   ^
     \        /     |
      \      /      HL
    22 \    / 21    |
        \  /        |
         10(BLU)    |
         
Tens digit:
  1 = nodes with ultrasound rangefinders (Node_Location)
  2 = nodes with RGB LED strips (Node_Light)

Ones digit (with same tens digit):
  +1 (modulo) = "to my right"/next
  -1 (modulo) = "to my left"/previous

*/
/*
                    // GRN, YEL, BLU  
const byte PIN_PW[] = {21, 17, 12};
const byte PIN_RNG[] = {20, 16, 11};
const byte PIN_EN[] = {22, 18, 13};
// hinky #defines to pacify FastLEDs constant expression neediness.
#define PIN_DATA0 19
#define PIN_DATA1 14
#define PIN_DATA2 10
const byte PIN_DATA[] = {PIN_DATA0, PIN_DATA1, PIN_DATA2};
*/
                    // BLU, YEL, GRN  
const byte PIN_PW[] = {12, 17, 21};
const byte PIN_RNG[] = {11, 16, 20};
const byte PIN_EN[] = {13, 18, 22};
// hinky #defines to pacify FastLEDs constant expression neediness.
#define PIN_DATA0 10
#define PIN_DATA1 14
#define PIN_DATA2 19
const byte PIN_DATA[] = {PIN_DATA0, PIN_DATA1, PIN_DATA2};


/*

// 22,21,20,19
#define C0_PW   21    // wire to Sonar PW via shifter
#define C0_RNG  20    // wire to Sonar RX via shifter
#define C0_EN   22    // wire to Sonar +5 via shifter
#define C0_DATA 19    // wire to lighting DATA IN via shifter

// 18,17,16,14
// skipping D15; has an LED on that pin.
#define C1_PW   17    // wire to Sonar PW via shifter
#define C1_RNG  16    // wire to Sonar RX via shifter
#define C1_EN   18    // wire to Sonar +5 via shifter
#define C1_DATA 14    // wire to lighting DATA IN via shifter

// 13,12,11,10
#define C2_PW   12    // wire to Sonar PW via shifter
#define C2_RNG  11    // wire to Sonar RX via shifter
#define C2_EN   13    // wire to Sonar +5 via shifter
#define C2_DATA 10    // wire to lighting DATA IN via shifter
*/

class Location {
  public:
    // startup
    void begin(Distances *D);

    // run range finding
    void update(byte smoothing=3);

    // given the distances to the object, calculate the location of the object and push to D
    void calculateLocation();
   
    // storage.  made external for others' use. 
    unsigned long currRange[N_RANGE]; // value of last range
    unsigned long avgRange[N_RANGE]; // smoothed ranges

  private:
    byte left(byte i);
    byte right(byte i);

    // store a pointer to the distance object
    Distances * d;

    // helper functions
    unsigned long squared(word x);
    word squareRoot(unsigned long x);
    unsigned long safeSub(unsigned long a, unsigned long b);
    unsigned long safeSub(unsigned long a, word b);
    word safeSub(word a, word b);
    
    word semiPerimeter(byte i);
    word altitudeHeight(byte i);
    void correctAltitudeHeight();
    word altitudeBase(byte i);
    word collinearBase(byte i);
    word collinearHeight(byte i);
    word area(byte i);

    // sensor function
    void boot(byte sIndex, byte EN_PIN, byte RNG_PIN, byte PW_PIN);


};

#endif

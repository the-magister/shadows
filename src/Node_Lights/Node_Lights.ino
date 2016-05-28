#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <Network.h>

#include <FastLED.h>
#include <FiniteStateMachine.h>

#include "Animation.h"

// track our response using a finite state machine
// idle <--> outPlane <--> inPlane
State idle = State(idleUpdate); // nothing going on
State outPlane = State(outPlaneUpdate); // sensors are picking up an object, but not within the plane of the triangle
State inPlane = State(inPlaneUpdate); // sensors are picking up an object, and it's within the place of the triangle
FSM S = FSM(idle); // start idle

// define the extents of the plane
#define MIN_X   -BASE_LEN/2
#define MID_X   0
#define MAX_X   BASE_LEN/2

#define MIN_Y   0
#define MID_Y   HEIGHT_LEN/2
#define MAX_Y   HEIGHT_LEN

// sparking levels 
#define MAX_SPARK   200
#define MIN_SPARK   50

// index accessor
#define MY_I   N.whoAmI()-20

void setup() {
  // give the distance sensors enough time to calibrate and start sending meaningful x,y calculations
  delay(1000);
  
  Serial.begin(115200);

  // start the radio
  N.begin();

  // startup animation
  A.begin();
}

void loop() {
  // update the radio traffic
  N.update();

  // update the FSM
  S.update();

  // update the animation
  A.update();
}

void idleUpdate() {
  // drive the shadow back to the middle
  A.fadePositionTo(mapXtoPixel(MID_X));

  // drive the heat back to baseline
  A.fadeSparksTo(MIN_SPARK);

  // check for state changes

  // do we detect something out there?
  if ( N.msg.d[MY_I] < MAX_Y*2 ) {
    S.transitionTo( outPlane );
  }
}


void outPlaneUpdate() {
  // drive the shadow back to the middle
  A.fadePositionTo(mapXtoPixel(MID_X));

  // drive the heat up or down, depending on distance sensors
  A.fadeSparksTo(mapDtoSpark(N.msg.d[MY_I] - MAX_Y));

  // check for state changes

  // do we detect nothing out there?
  if ( N.msg.d[MY_I] > MAX_Y*2 ) {
    S.transitionTo( idle );
  }

  // do we detect something in the plane?
  if ( N.msg.d[0] < MAX_Y && N.msg.d[1] < MAX_Y && N.msg.d[2] < MAX_Y ) {
    S.transitionTo( inPlane );
  }
}

void inPlaneUpdate() {
  // pull my x, y and distance information
  int xp = projectObjectX(N.msg.x[MY_I], N.msg.y[MY_I]);
  int dp = projectObjectDistance(N.msg.d[MY_I], N.msg.y[MY_I]);

  // drive shadow location according to x'
  A.fadePositionTo(mapXtoPixel(xp));

  // drive the heat according to distance d'
  A.fadeSparksTo(mapDtoSpark(dp));
  
  // check for state changes

  // do we detect something, but out of the plane?
  if ( N.msg.d[0] > MAX_Y || N.msg.d[1] > MAX_Y || N.msg.d[2] > MAX_Y ) {
    S.transitionTo( outPlane );
  }
}

// helper function to map location to pixels
/* Diagram of the pixel location as a function of the object location

Location:  left edge ----------------- right edge
X:         MIN_X              0             MAX_Y
Pixel:     0                                NUM_LEDS-1 

 */
byte mapXtoPixel(int x) {
  // map with constraint
  return(
    map(
      constrain(x, MIN_X, MAX_X),  // constrain x to be [MIN_X, MAX_X]
      MIN_X, MAX_X, // map [MIN_X, MAX_X]
      0, NUM_LEDS-1 // to [0, NUM_LEDS-1]
    )
  );
}

// helper function to map distance to spark intensity
/*  Diagram of the spark intensity as a function of distance to plane:

State:       /----- inPlane -------\     /---- outPlane -----\          /----- idle -----\        
Location: Sensor ---------------> Plane Edge ------------> Edge of Detection ----------> No Detection
Y:         MAX_Y        MID_Y         MIN_Y                     MAX_Y*2
Spark:     MIN_SPARK                 MAX_SPARK                     MIN_SPARK                    MIN_SPARK
                
 */
byte mapDtoSpark(byte d) {
  // map with constraint
  return(
      map(
        constrain(d, MIN_Y, MAX_Y), // constrain d to be [MIN_Y, MAX_Y]
        MIN_Y, MAX_Y, // map [MIN_Y, MAX_Y]
        MAX_SPARK, MIN_SPARK
      )
  );
}

// helper function to project object (x,y) to (x',0); returns x'
int projectObjectX(int x, int y) {
  // tanget rule for right triangles, without all of the tedious angles
  float projX = (float)x * (float)MAX_Y/((float)MAX_Y - (float)y);
  return( projX );
}

// helper function to return distance from object (x,y) to (x',0); returns distance 
int projectObjectDistance(int d, int y) {
  // cosine rule for right triangles, without all of the tedious angles
  float projD = (float)d * ( (float)MAX_Y/((float)MAX_Y - (float)y) - 1.0 );
  return( projD );
}





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

// intensity levels 
#define MAX_INTENSITY   255
#define MIN_INTENSITY   0

// index accessor
#define MY_I   N.whoAmI()-20

void setup() {

  Serial.begin(115200);

  Serial << F("main.  location calibrations: ");
  Serial << F("\tX=") << 0 << F(" -> P=") << mapXtoPixel(0);
  Serial << F("\tX=") << BASE_LEN/2 << F(" -> P=") << mapXtoPixel(BASE_LEN/2);
  Serial << F("\tX=") << BASE_LEN << F(" -> P=") << mapXtoPixel(BASE_LEN);
  Serial << endl;
  
  // start the radio
  N.begin();

  // give the distance sensors enough time to calibrate and start sending meaningful x,y calculations
  // AND enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while(! startupDelay.check()) N.update();

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
  A.fadePositionTo(mapXtoPixel(BASE_LEN/2));

  // drive the intensity back to baseline
  A.fadeIntensityTo(MIN_INTENSITY);

  // check for state changes

  // do we detect something out there?
  if ( N.objectAnywhere() ) {
    N.printMessage();
    Serial << F("State.  idle->outPlane.") << endl;
    S.transitionTo( outPlane );
  }
}


void outPlaneUpdate() {
  // drive the shadow back to the middle
  A.fadePositionTo(mapXtoPixel(BASE_LEN/2));

  // drive the heat up or down, depending on distance sensors
  A.fadeIntensityTo(mapDtoIntensity(N.msg.d[MY_I] - BASE_LEN));

  // check for state changes

  // do we detect nothing out there?
  if ( ! N.objectAnywhere() ) {
    N.printMessage();
    Serial << F("State.  outPlane->idle.") << endl;
    S.transitionTo( idle );
  }

  // do we detect something in the plane?
  if ( N.objectInPlane() ) {
    N.printMessage();
    Serial << F("State.  outPlane->inPlane.") << endl;
    S.transitionTo( inPlane );
  }
}

void inPlaneUpdate() {
  // drive shadow location according to x'
  A.fadePositionTo(mapXtoPixel(N.intercept()));

  // drive the intensity according to distance d'
  A.fadeIntensityTo(mapDtoIntensity(N.range()));

  // check for state changes

  // do we detect something, but out of the plane?
  if ( ! N.objectInPlane() ) {
    N.printMessage();
    S.transitionTo( outPlane );
    Serial << F("State.  inPlane->outPlane.") << endl;
  }
}

// helper function to map location to pixels
/* Diagram of the pixel location as a function of the object location

Location:  left edge ----------------- right edge
X:         0                             BASE_LEN
Pixel:     0                           NUM_LEDS-1 

 */
byte mapXtoPixel(int x) {
  // map with constraint
  return(
    map(
      constrain(x, 0, BASE_LEN),  // constrain x to be [0, BASE_LEN]
      0, BASE_LEN, // map [0, BASE_LEN]
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
byte mapDtoIntensity(byte d) {
  // map with constraint
  return(
      map(
        constrain(d, 0, HEIGHT_LEN), // constrain d to be [MIN_Y, MAX_Y]
        0, HEIGHT_LEN, // map [0, HEIGHT_LEN]
        MAX_INTENSITY, MIN_INTENSITY
      )
  );
}



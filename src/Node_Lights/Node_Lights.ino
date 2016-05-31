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

void setup() {

  Serial.begin(115200);

  Serial << F("main.  location calibrations: ");
  Serial << F("\tX=") << 0 << F(" -> P=") << mapXtoPixel(0);
  Serial << F("\tX=") << BASE_LEN/2 << F(" -> P=") << mapXtoPixel(BASE_LEN/2);
  Serial << F("\tX=") << BASE_LEN << F(" -> P=") << mapXtoPixel(BASE_LEN);
  Serial << endl;
  
  // start the radio
  N.begin();

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while(! startupDelay.check()) N.update();

  // startup animation
  A.begin();

}

void loop() {
  // update the radio traffic
  if( N.update() ) N.printMessage();

  // update the FSM
  S.update();

  // update the animation
  A.update();

}

void idleUpdate() {
  // drive the intensity back to baseline
  A.setIntensity( 0 );
 
  // check for state changes

  // do we detect something out there?
  if ( N.objectAnywhere() ) {
    N.printMessage();
    Serial << F("State.  idle->outPlane.") << endl;
    S.transitionTo( outPlane );
    A.setAnimation( A_OUTPLANE, false );
  }
}


void outPlaneUpdate() {
  // drive the heat up or down, depending on distance sensors
  A.setIntensity( 255-N.distance() );

  // check for state changes

  // do we detect nothing out there?
  if ( ! N.objectAnywhere() ) {
    N.printMessage();
    Serial << F("State.  outPlane->idle.") << endl;
    S.transitionTo( idle );
    A.setAnimation( A_IDLE, false );
  }

  // do we detect something in the plane?
  if ( N.objectInPlane() ) {
    N.printMessage();
    Serial << F("State.  outPlane->inPlane.") << endl;
    S.transitionTo( inPlane );
    A.setAnimation( A_INPLANE );
  }
}

void inPlaneUpdate() {
  // drive shadow location according to intercept
  A.setPosition(mapXtoPixel( N.intercept() ));

  // drive the intensity according to range
  A.setIntensity(mapDtoIntensity( N.range() ));

  // check for state changes

  // do we detect something, but out of the plane?
  if ( ! N.objectInPlane() ) {
    N.printMessage();
    Serial << F("State.  inPlane->outPlane.") << endl;
    S.transitionTo( outPlane );
    A.setAnimation( A_OUTPLANE );
  }
}

// helper function to map location to pixels
/* Diagram of the pixel location as a function of the object location

Location:  left edge ----------------- right edge
X:         0                             BASE_LEN
Pixel:     0                           NUM_LEDS-1 

 */
byte mapXtoPixel(word x) {
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
/*  Diagram of the intensity as a function of distance to plane:

Location: Sensor ---------------> Plane Edge
distance: 0            [HEIGHT_LEN, BASE_LEN]
range:    [HEIGHT_LEN, BASE_LEN]           0              
                
 */
byte mapDtoIntensity(word r) {
  // map with constraint
  return(
      map(
        constrain(r, 0, BASE_LEN), // constrain r to be [0, BASE_LEN]
        0, BASE_LEN, // map [0, BASE_LEN]
        255, 0 // to [255,0]
      )
  );
}



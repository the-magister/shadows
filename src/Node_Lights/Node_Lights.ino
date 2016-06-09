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

}

void idleUpdate() {
  // drive the intensity back to baseline
  A.setIntensity( 64 );
 
  // check for state changes

  // do we detect something out there?
  if ( N.objectAnywhere() ) {
    N.printMessage();
    Serial << F("State.  idle->outPlane.") << endl;
    S.transitionTo( outPlane );
    A.setAnimation( A_OUTPLANE, false );
  }

  // update the animation
  A.update();
}


void outPlaneUpdate() {
  // drive the brightness up or down, depending on distance sensors
  A.setIntensity( 
    map(
      constrain(N.distance(), P_EDGE_RANGE, P_EDGE_RANGE-P_EDGE_TRI), 
      P_EDGE_RANGE, P_EDGE_RANGE-P_EDGE_TRI, // [edge of detectable range, edge of triangle]
      64, 255 // [dim, bright]
    )
  );

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

  // update the animation
  A.update();
}

void inPlaneUpdate() {
  // drive shadow location according to intercept
  A.setPosition(
    map(
      constrain(N.intercept(), 0, BASE_LEN),  // constrain x to be [0, BASE_LEN]
      0, BASE_LEN, // map [0, BASE_LEN]
      0, NUM_LEDS-1 // to [0, NUM_LEDS-1]
    )
  );

  // drive the intensity according to range
  A.setIntensity(
    map(
      constrain(N.range(), 0, BASE_LEN),
      0, BASE_LEN, // map [0, BASE_LEN]
      0, NUM_LEDS-1 // to [0, NUM_LEDS-1]
    )
  );

  // check for state changes

  // do we detect something, but out of the plane?
  if ( ! N.objectInPlane() ) {
    N.printMessage();
    Serial << F("State.  inPlane->outPlane.") << endl;
    S.transitionTo( outPlane );
    A.setAnimation( A_OUTPLANE );
  }

  // update the animation
  A.update();


}


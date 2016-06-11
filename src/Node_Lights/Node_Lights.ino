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
  
  // update the animation
  A.update();
}

void idleUpdate() {
  // drive the intensity back to baseline
  A.setIntensity( 255 );
 
  // check for state changes

  // do we detect something out there?
  if ( N.objectInPlane() ) {
    N.printMessage();
    Serial << F("State.  idle->outPlane.") << endl;
    S.transitionTo( inPlane );
    A.setAnimation( A_INPLANE, false );
  }
}

void inPlaneUpdate() {
  // drive shadow location according to intercept
  A.setPosition(
    map(
      constrain(N.myIntercept(), 0, BASE_LEN),  // constrain x to be [0, BASE_LEN]
      BASE_LEN, 0, // map [0, BASE_LEN]
      0, NUM_LEDS-1 // to [0, NUM_LEDS-1]
    )
  );

  // drive the intensity according to range
  A.setIntensity(
    map(
      constrain(N.myRange(), 0, BASE_LEN),
      0, BASE_LEN, // map [0, BASE_LEN]
      0, NUM_LEDS-1 // to [0, NUM_LEDS-1]
    )
  );

  // check for state changes

  // do we detect something, but out of the plane?
  if ( ! N.objectInPlane() ) {
    N.printMessage();
    Serial << F("State.  inPlane->idle.") << endl;
    S.transitionTo( idle );
    A.setAnimation( A_IDLE );
  }

}


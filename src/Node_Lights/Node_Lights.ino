#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <FastLED.h>
#include <FiniteStateMachine.h>

// Shadows specific libraries.  
#include <Network.h>
Network N;
Distances D;

#include "Animation.h"

// track our response using a finite state machine
// idle <--> outPlane <--> inPlane
void idleUpdate();
State idle = State(idleUpdate); // nothing going on
void inPlaneUpdate();
State inPlane = State(inPlaneUpdate); // sensors are picking up an object, and it's within the place of the triangle
FSM S = FSM(idle); // start idle

const word distInPlane = 0.9*HL;

// track the average of Ch and Cb
unsigned long Ch = HL;
unsigned long Cb = HL;
byte window = 3; // how many sensor measurements do we average to smooth Cb and Ch?
// each read is about 17 ms, 
// so the for three sensors of updates, we're talking ~50ms. 

// track state
systemState lastState;

void setup() {

  Serial.begin(115200);

  // start the radio
  N.begin(&D);

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  // startup animation
  A.begin();

}

boolean objectInPlane() {
  return(
    D.D[0] <= distInPlane || D.D[1] <= distInPlane || D.D[2] <= distInPlane
//      true
  );
}

void loop() {
  // update the radio traffic
  if( N.update() ) {
    // running average
    Cb = ((window-1)*Cb + D.Cb[N.myIndex])/window;
    Ch = ((window-1)*Ch + D.Ch[N.myIndex])/window;

//    Serial << F("Cb=") << L.Cb[N.myIndex] << F(" avg=") << Cb;
//    Serial << F("\tCh=") << L.Ch[N.myIndex] << F(" avg=") << Ch;
//    Serial << endl;
  }

  // check for system mode changes
  if ( N.state != lastState ) {
    if ( N.state == M_CALIBRATE ) {
      A.setAnimation(A_CALIBRATE, false);
    } else if ( N.state == M_PROGRAM ) {
      A.setAnimation(A_PROGRAM, false);
    } else if ( N.state == M_NORMAL ) {
      S.transitionTo( idle );
      A.setAnimation( A_IDLE, false );
    }
    lastState = N.state;
  }
  
  // update the FSM, letting it set animations as needed.
  S.update();

  // update the animation
  A.update();
}

void idleUpdate() {
  // until we get normalized, stay idle
  if ( N.state != M_NORMAL ) return;

  // check for state changes
  static Metro goInPlaneTimeout(500UL);
  if ( ! objectInPlane() ) goInPlaneTimeout.reset();

  // do we detect something out there?
  if ( goInPlaneTimeout.check() ) {
    Serial << F("State.  idle->inPlane.") << endl;
    S.transitionTo( inPlane );
    A.setMasterBrightness( 255 ); // restore brightness
    A.setAnimation( A_INPLANE, false );
  }
}

void inPlaneUpdate() {
  // until we get normalized, stay idle
  if ( N.state != M_NORMAL ) return;
  
//  Serial << F("L.Cb=") << L.Cb[N.myIndex] << F("\tL.Ch=") << L.Ch[N.myIndex] << endl;

  // drive shadow center according to intercept
  A.setCenter(
    map(
//      constrain(L.Cb[N.myIndex], 0, SL),  // constrain x to be [0, SL]
      constrain(Cb, 0, SL),  // constrain x to be [0, SL]
      0, SL, // map [0, SL]
      0, NUM_LEDS - 1 // to [0, NUM_LEDS-1]
    )
  );

  // drive shadow extent according to range
  A.setExtent(
    map(
//      constrain(L.Ch[N.myIndex], 0, SL),
      constrain(Ch, 0, HL),
      0, HL, // map [0, HL]
      0, NUM_LEDS / 2 // to [0, NUM_LEDS/2]
    )
  );

  // check for state changes
  static Metro goIdleTimeout(500UL);
  if ( objectInPlane() ) goIdleTimeout.reset();

  // nothing to see here or no longer in normal operation?
  if ( goIdleTimeout.check() || N.state != M_NORMAL ) {
    Serial << F("State.  inPlane->idle.") << endl;
    S.transitionTo( idle );
    A.setAnimation( A_IDLE, false );
  }

}


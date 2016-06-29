#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <Network.h>

#define NI 3

void setup() {

  Serial.begin(115200);

  // start the radio
  N.begin(21);

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  randomSeed(analogRead(4));

  test_s(); // should pass with i=[0,3]

  test_d(0, 1023); // should all pass
  test_d(1024, 65534); // should all fail

  test_center();

  test_corner();

  Serial << F("====\nTests complete") << endl;
}

void loop() {
  N.update();
}

// make sure we can correctly encode and decode s
void test_s() {
  Serial << F("==== TEST: encoding and decoding s") << endl;
  for (byte i = 0; i <= 5; i++) {
    N.s = i;
    Serial << F("SOLN: s=") << N.s;
    // encode
    N.encodeMessage();
    N.decodeMessage();
    if ( N.s != i) {
      Serial << F("\tFAIL\t");
    } else {
      Serial << F("\tPASS\t");
    }
    N.showMessage();
  }
}

void test_d(word low, word high) {
  Serial << F("==== TEST: encoding and decoding distances from:") << low << F(" to:") << high << endl;
  N.s = 0;
  
  for (byte i = 0; i < 10; i++) {
    word soln[NI];
    for (byte n = 0; n < NI; n++) {
      soln[n] = random(low, high);
      N.distance[n] = soln[n];
    }
    Serial << F("SOLN: ");
    N.showMessage();

    N.encodeMessage();
    N.decodeMessage();
    if ( memcmp(soln, N.distance, sizeof(N.distance)) != 0 ) {
      Serial << F("FAIL\t");
    } else {
      Serial << F("PASS\t");
    }
    N.showMessage();
  }


}

unsigned long elapsedTime() {
  static unsigned long then = micros();
  unsigned long now = micros();
  unsigned long delta = now - then;
  then = now;
  return ( delta );
}

boolean compare(word soln[NI], word calc[NI]) {
  Serial << F("soln\t") << F("\t") << soln[0]  << F("\t") << soln[1]  << F("\t") << soln[2] << endl;
  Serial << F("calc\t") << F("\t") << calc[0]  << F("\t") << calc[1]  << F("\t") << calc[2] << endl;
 
  if ( memcmp(soln, calc, sizeof(calc)) != 0 ) {
    return( false );
  } else {
    return( true );
  }
}

boolean compare(word soln, word calc) {
  Serial << F("soln\t") << F("\t") << soln << endl;
  Serial << F("calc\t") << F("\t") << calc << endl;
 
  if ( soln != calc ) {
    return( false );
  } else {
    return( true );
  }
}

void test_center() {
  Serial << F("==== TEST: center") << endl;
  Serial << F("Indexes: lI=") << N.lI << F("\tmI=") << N.mI << F("\trI=") << N.rI << endl;

  // softball test.  object is nearly in the center
  N.s = 3;
  N.distance[0] = 476;
  N.distance[1] = 630;
  N.distance[2] = 283;
  N.encodeMessage();

  // track the time it takes for everything.
  unsigned long tic = elapsedTime();
  N.decodeMessage();
  tic = elapsedTime();
  Serial << F("N.decodeMessage() time (us)=") << tic << endl;
 
  N.showMessage();
  
  Serial << endl << F("Ab:") << endl;
  word Absoln[NI] = {587,280,264};
  Serial << F("result: ") << compare(Absoln, N.Ab) << endl;

  Serial << endl << F("Ah:") << endl;
  word Ahsoln[NI] = {229, 41, 396};
  Serial << F("result: ") << compare(Ahsoln, N.Ah) << endl;

  Serial << endl << F("mCb:") << endl;
  word Cbsoln[NI] = {684, 276, 114};
  Serial << F("result: ") << compare(Cbsoln[N.mI], N.mCb) << endl;

  Serial << endl << F("mCh:") << endl;
  word Chsoln[NI] = {249, 41, 423};
  Serial << F("result: ") << compare(Chsoln[N.mI], N.mCh) << endl;

  Serial << endl << F("mArea:") << endl;
  word Areasoln[NI] = {19877, 3558, 34373};
  Serial << F("result: ") << compare(Areasoln[N.mI], N.mArea) << endl;

}


void test_corner() {
  Serial << F("==== TEST: corner") << endl;
  Serial << F("Indexes: lI=") << N.lI << F("\tmI=") << N.mI << F("\trI=") << N.rI << endl;

  // hard test.  object is nearly in the corner
  N.s = 3;
  N.distance[0] = 61;
  N.distance[1] = 655;
  N.distance[2] = 655;
  N.encodeMessage();
  
  // track the time it takes for everything.
  unsigned long tic = elapsedTime();
  N.decodeMessage();
  tic = elapsedTime();
  Serial << F("N.decodeMessage() time (us)=") << tic << endl;

  N.showMessage();
  
  Serial << endl << F("Ab:") << endl;
  word Absoln[NI] = {377,659,95};
  Serial << F("result: ") << compare(Absoln, N.Ab) << endl;

  Serial << endl << F("Ah:") << endl;
  word Ahsoln[NI] = {536, 60, 60};
  Serial << F("result: ") << compare(Ahsoln, N.Ah) << endl;

  Serial << endl << F("mCb:") << endl;
  word Cbsoln[NI] = {377, 678, 76};
  Serial << F("result: ") << compare(Cbsoln[N.mI], N.mCb) << endl;

  Serial << endl << F("mCh:") << endl;
  word Chsoln[NI] = {536,63,63};
  Serial << F("result: ") << compare(Chsoln[N.mI], N.mCh) << endl;

  Serial << endl << F("mArea:") << endl;
  word Areasoln[NI] = {46525, 5208, 5208};
  Serial << F("result: ") << compare(Areasoln[N.mI], N.mArea) << endl;

}


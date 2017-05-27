#include <Streaming.h>
#include <Metro.h>
#include <wavTrigger.h>
#include <SoftwareSerial.h>
#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>
#include <FastLED.h>

// Shadows specific libraries.
#include <Network.h>
Network N;
Distances D;
#include "Location.h"
Location L;

#include "Sound.h"
SoftwareSerial wavSerial(WAVE_TX, WAVE_RX);
Sound S;

// Moteuino R4 pins already in use:
//  D2, D10-13 - transceiver
//  D9         - LED
//  D8, D11-13 - flash

#define LED_TYPE WS2801
#define COLOR_ORDER RGB
#define NUM_LEDS_PER_CORNER 2
#define NUM_LEDS NUM_LEDS_PER_CORNER*N_RANGE
CRGB leds[NUM_LEDS];
// index to the correct nodes
const byte cornerIndex[N_RANGE] = {NUM_LEDS_PER_CORNER*0, NUM_LEDS_PER_CORNER*1, NUM_LEDS_PER_CORNER*2};
#define PIN_LED_CLK 3    // corner LED clock line
#define PIN_LED_DATA 4    // corner LED data line
// color correction options; see FastLED/color.h
// #define COLOR_CORRECTION TypicalLEDStrip
#define COLOR_CORRECTION TypicalSMD5050

#define DEBUG_UPDATE 0
#define DEBUG_DISTANCE 0
#define DEBUG_ALTITUDE 0
#define DEBUG_COLLINEAR 0
#define DEBUG_AREA 0
#define DEBUG_INTERVAL 0

void setup() {
  // Need to make sure do this immediately after startup.
  // can't let the range finders go into "free ranging" mode or
  // the calibration will get crappy.
  digitalWrite(PIN_START_RANGE, LOW);
  pinMode(PIN_START_RANGE, OUTPUT);
  
  Serial.begin(115200);

  // start the radio
  N.begin(&D, 255, GROUPID, RF69_915MHZ, 5); // Higher power setting goofing analogRead()!!

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  // start the range finder
  L.begin(&D);

  // start the sound
  S.begin(&D, &wavSerial);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,PIN_LED_DATA,PIN_LED_CLK,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(COLOR_CORRECTION);
  // set master brightness control
  FastLED.setBrightness(255);
}

void loop() {
  // update the radio traffic
  boolean haveTraffic = N.update();
  static Metro backToNormalAfterProgram(5000UL);
  if( haveTraffic && N.state==M_PROGRAM ) {
    backToNormalAfterProgram.reset();
  }

  // configure to calibrate once
  if ( N.state == M_CALIBRATE ) {   
     L.begin(&D);
     delay(100);
     N.state = M_NORMAL;
  }

  if ( haveTraffic ) {
    Serial << F("RECV: "); N.showNetwork();
  }

  // @ KiwiBurn noted need for periodic reboot to the lights
  static Metro rebootEvery(30UL * 1000UL);
  if( objectInPlane() ) rebootEvery.reset();
  if( rebootEvery.check() ) {
    rebootEvery.reset();
    N.state = M_REBOOT;
    Serial << F("Reboot timer expired.  Rebooting.") << endl;
    for(byte i=0;i<10; i++) {
      N.sendState();
      delay(5);
    }  
//    resetUsingWatchdog(true);
  }

  // average the sensor readings
  L.update();

  // update sound
  S.update();

  // update lights
  static CHSV color(HUE_RED, 255, 255);
  color.hue += 1;
  if( N.state == M_PROGRAM ) color.hue = HUE_BLUE; // show we're programming
  for( byte i=0; i<N_RANGE; i++ ) {
    // assign value from distance
    byte distance = map(D.D[i], 0, HL, 0, 255-12);
    for( byte j=0; j<NUM_LEDS_PER_CORNER; j++ ) {
      leds[cornerIndex[i]+j] = color;
      leds[cornerIndex[i]+j].fadeLightBy( distance );
    }
  }
  FastLED.show();

  // send
  if( N.state == M_PROGRAM ) {
    // just don't want to be locked out forever
    if( backToNormalAfterProgram.check() ) N.state = M_NORMAL;
    return;
  }

  // send
  N.sendMessage();
  
  if( DEBUG_UPDATE ) {
    for ( byte i = 0; i < N_RANGE; i++ ) {
      Serial << F("S") << i << F(" reading=") << L.reading[i] << F("\t");
    }
    Serial << endl;
  }
  
  if( DEBUG_DISTANCE ) {
    for ( byte i = 0; i < N_RANGE; i++ ) {
      Serial << F("S") << i << F(" range=") << D.D[i] << F("\t");
    }
    Serial << endl;
  }

  if( DEBUG_ALTITUDE ) {
    for ( byte i = 0; i < N_NODES; i++ ) {
      Serial << F("N") << i << F(" Ab=") << D.Ab[i] << F("\t") << F("Ah=") << D.Ah[i] << F("\t\t");
    }
    Serial << endl;
  }
  
  if( DEBUG_COLLINEAR ) {
    for ( byte i = 0; i < N_NODES; i++ ) {
      Serial << F("N") << i << F(" Cb=") << D.Cb[i] << F("\t") << F("Ch=") << D.Ch[i] << F("\t\t");
    }
    Serial << endl;
  }

  if( DEBUG_AREA ) {
    for ( byte i = 0; i < N_NODES; i++ ) {
      Serial << F("N") << i << F(" Area=") << D.Area[i] << F("\t\t");
    }
    Serial << endl;
  }

  static unsigned long tic = millis();
  static unsigned long toc = millis();
  if( DEBUG_INTERVAL ) {
     tic = toc;
     toc = millis();
     Serial << F("Update interval (ms)=") << toc - tic << endl;
  }

}

boolean objectInPlane() {

  const word distInPlane = 0.7*(float)HL;

  byte inPlane0 = D.D[0] <= distInPlane;
  byte inPlane1 = D.D[1] <= distInPlane;
  byte inPlane2 = D.D[2] <= distInPlane;

  return( 
    (inPlane0 + inPlane1 + inPlane2) >= 2
  );
}


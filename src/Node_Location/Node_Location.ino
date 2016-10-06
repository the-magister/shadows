#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

// Shadows specific libraries.
#include <Network.h>
Network N;
Distances D;
#include "Location.h"
Location L;

// Moteuino R4 pins already in use:
//  D2, D10-13 - transceiver
//  D9         - LED
//  D8, D11-13 - flash

#define PIN_LED_CLK 3    // corner LED clock line
#define PIN_LED_DATA 4    // corner LED data line

#define DEBUG_UPDATE 0
#define DEBUG_DISTANCE 1
#define DEBUG_ALTITUDE 0
#define DEBUG_COLLINEAR 0
#define DEBUG_AREA 0
#define DEBUG_INTERVAL 0

void setup() {
  Serial.begin(115200);

  // start the radio
  N.begin(&D);

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  // start the range finder
  L.begin(&D);
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

  // average the sensor readings
  L.update();  

  // send
  if( N.state == M_PROGRAM ) {
    // just don't want to be locked out forever
    if( backToNormalAfterProgram.check() ) N.state = M_NORMAL;
    return;
  }

  // send
  N.sendMessage();

  if( DEBUG_DISTANCE ) {
    for ( byte i = 0; i < N_RANGE; i++ ) {
      Serial << F("S") << i << F(" range(units)=") << D.D[i] << F("\t");
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
  
}

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

}

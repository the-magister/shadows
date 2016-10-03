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
#include <Distance.h>

void setup() {
  Serial.begin(115200);

  // start the radio
  N.begin();

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  // start the range finder
  D.begin();
}

void loop() {
  // update the radio traffic
  boolean haveTraffic = N.update();

  // configure to calibrate once
  if ( N.state == M_CALIBRATE ) {   
     D.begin();
     delay(100);
     N.state = M_NORMAL;
  }

  if ( haveTraffic ) {
    Serial << F("RECV: "); N.showNetwork();
  }

  // average the sensor readings
  static unsigned long distanceAvg[N_RANGE];

  // update distance
  static Metro nyquistUpdate(1);
  if( nyquistUpdate.check() ) {
    if( D.update() ); // let us know, or something.      
    for( byte i=0; i<N_RANGE; i++ ) {
      distanceAvg[i] = distanceAvg[i] * 9 + D.distance[i];
    }
  }

  // send
  static Metro sendInterval(30);
  if( sendInterval.check() ) {  

    if( N.state == M_PROGRAM ) {
      // just don't want to be locked out forever
      N.state = M_NORMAL;
      return;
    }

    // encode
    for( byte i=0; i<N_RANGE; i++ ) {
      N.distance[i] = constrain(distanceAvg[i], 0, 255);
    }
    
    // send
    N.sendMessage();
    
    // show
    Serial << F("SEND: "); N.showNetwork();
    
    // reset the timer
    sendInterval.reset();
    
  } 
}


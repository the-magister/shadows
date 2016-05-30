#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <Network.h>

#include "Location.h"

void setup() {
  // need to very quickly prevent calibration on the range sensor, as we don't want them calibrating at the same time (interference)
  digitalWrite(RANGE_PIN, LOW);
  pinMode(RANGE_PIN, OUTPUT);
  
  Serial.begin(115200);

  // start the radio
  N.begin();

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while(! startupDelay.check()) N.update();

  // start the range finder
  L.begin(N.whoAmI());
}

void loop()
{
  // update the radio traffic
  N.update();

  // do I need to update the position information?
  if ( N.meNext() ) {
    // update distance
    L.readDistance( N.msg );

    // calculate positions
    L.calculatePosition( N.msg );

    // send
    N.send();

    // show
    N.printMessage();
  }
}


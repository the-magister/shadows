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

void setup() {
  Serial.begin(115200);

  // start the range finder
  L.begin(&D);

}

void loop() {

  // average the sensor readings
  L.update();

  for ( byte i = 0; i < N_RANGE; i++ ) {
    Serial << F("S") << i << F(" range(units)=") << D.D[i] << F("\t");
  }
  Serial << endl;

  delay(100); // stop spam

  while(digitalRead(PIN_START_RANGE)==LOW);
  unsigned long tic = millis();
  delay(5);
  while(digitalRead(PIN_START_RANGE)==LOW);
  unsigned long toc = millis();  

  Serial << F("update interval (ms)=") << toc - tic << endl;
  // seems to take 128 ms to get a round-trip.  ~42 ms per sensor, which is about right.
}

// readings: 212 ie. 60"

// Analog, (Vcc/512) / inch

// 212/1023
// reading : 0.2 V

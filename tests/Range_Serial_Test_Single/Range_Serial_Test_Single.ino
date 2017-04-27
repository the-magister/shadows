// add the following to Preferences->Board Manager URL
// Add Moteino to Board Manager
// Compile for Moteino Mega

/* 
 *  pinout: https://www.flickr.com/photos/15304611@N03/14677033948
 *  
 *  LED: D15
 *  Transceiver: D2, D4-7
 *  Flash: D5-7, 23
 */
 
#define LED 13

#include <Streaming.h>
#include <Metro.h>
#include <SoftwareSerial.h>

#define RX0 10
#define TX0 9
SoftwareSerial range0(TX0, RX0, true); // TX, RX on sonar 0.

void setup() {
  // LED
  pinMode(LED, OUTPUT);

  pinMode(RX0, OUTPUT);
  digitalWrite(RX0, HIGH);
  
  Serial.begin(115200);

  // 250mS after power-up, the LV-MaxSonar-EZ is ready to accept the RX command.
  delay(250); // delay after power up

  Serial << "Startup" << endl;

  Serial << "Startup 0." << endl;
  // calibrate
  range0.begin(9600);
  delay(100);

  
}

void loop() {
  if( range0.available() ) {
    Serial << ".";
  }
  
  if( range0.available() && range0.read() == 'R') {
    delay(5);
    Serial << "Range 0: " << range0.parseInt() << endl;
  }

}


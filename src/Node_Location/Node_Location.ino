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

#define CALIBRATION_INTERVAL 60000UL // calibrate every minute if there's nothing going on

#define RESEND_INTERVAL 1000UL

systemState lastState;
byte recvFromNodeID, transToNodeID;

#define IN_PLANE (654U-10U)

void setup() {
  Serial.begin(115200);

  // start the radio
  N.begin();

  // track order for round-robin
  recvFromNodeID = N.left(N.myNodeID);
  transToNodeID = N.right(N.myNodeID);

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  // start the range finder
  D.begin();

}

unsigned long elapsedTime() {
  static unsigned long then = micros();
  unsigned long now = micros();
  unsigned long delta = now - then;
  then = now;
  return( delta );
}

void loop() {
  // update the radio traffic
  boolean haveTraffic = N.update();
  
  if( haveTraffic ) {
    Serial << F("RX: ");
    N.showNetwork(); 
    N.decodeMessage(); // updating the other's distance information so I can relay it with mine.
  }

  // figure out if we need to act
  boolean shouldAct = haveTraffic && (N.senderNodeID == recvFromNodeID);

  // do I need to update the position information?
  if ( shouldAct ) {
    digitalWrite(LED, HIGH);

/*
    // if it's good to recalibrate (nothing sensed), do so.
    static Metro calibrationInterval(CALIBRATION_INTERVAL);
    if ( N.distance[0] < IN_PLANE || N.distance[1] < IN_PLANE || N.distance[2] < IN_PLANE ) {
      // something was detected
      calibrationInterval.reset();
    }
    if( calibrationInterval.check() || N.state == M_CALIBRATE ) {
      D.calibrate();
      N.state = M_NORMAL;
    }
*/
    // how much time is spent by the other nodes?
    static unsigned long otherTime = 0;
    otherTime = (9*otherTime + 1*elapsedTime())/10; // running average
    
    // update distance
    word dist = D.read();
//    N.distance[N.myIndex] = map(dist, 0, distanceToLEDs, 0, HL); // scale to correct for warp
    N.distance[N.myIndex] = dist;
    N.encodeMessage(); 
    
    // how much time is spent reading the sensors?
    static unsigned long sensorTime = 0;
    sensorTime = (9*sensorTime + 1*elapsedTime())/10; // running average
    
    // send
    boolean haveSent = false;
    N.showNetwork();  
    while( ! haveSent ) {
      Serial << F("Sending...") << endl;
      haveSent = N.sendMessage(transToNodeID);
    }
    Serial << F("ACKd.") << endl;

    // how much time is spent by sending data?
    static unsigned long sendTime = 0;
    sendTime = (9*sendTime + 1*elapsedTime())/10; // running average
 
    // show
    Serial << F("TX: "); N.showNetwork();

    static byte loopCount = 0;
    loopCount++;
    if( loopCount >= 10 ) {
      loopCount = 0;
      Serial << F("=== Timers:") << endl;
      Serial << F("otherTime (us)=") << otherTime << endl;
      Serial << F("sensorTime (us)=") << sensorTime << endl;
      Serial << F("sendTime (us)=") << sendTime << endl;
      Serial << F("===") << endl;
    }
    
    digitalWrite(LED, LOW);
  }

}


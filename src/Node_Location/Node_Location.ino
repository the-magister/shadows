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
boolean bootstrap = false;

#define IN_PLANE (654U-10U)

void setup() {
  Serial.begin(115200);

  // start the radio
  N.begin();

  // track order for round-robin
  recvFromNodeID = N.left(N.myNodeID);
  transToNodeID = N.right(N.myNodeID);
  Serial << F("Startup.  myNodeID=") << N.myNodeID << F("\trecvFrom=") << recvFromNodeID << F("\t transTo=") << transToNodeID << endl;

  // do I need to get things moving?
  if( N.myNodeID == 10 ) bootstrap = true;
  Serial << F("Bootstrap responsibility? ") << bootstrap << endl;
  
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
  }

  // figure out if we need to act
  boolean shouldAct = (N.state!=M_PROGRAM) && (haveTraffic) && (N.senderNodeID==recvFromNodeID);

  // do I need to update the position information?
  if ( shouldAct || bootstrap ) {
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

    N.decodeMessage(); // updating the other's distance information so I can relay it with mine.
    Serial << F("Decode Distances:\t") << N.distance[0] << F("\t") << N.distance[1] << F("\t") << N.distance[2] << endl;
    N.distance[N.myIndex] = dist;
    
    Serial << F("Update Distances:\t") << N.distance[0] << F("\t") << N.distance[1] << F("\t") << N.distance[2] << endl;
    N.encodeMessage(); 
    Serial << F("Encode Distances:\t") << N.distance[0] << F("\t") << N.distance[1] << F("\t") << N.distance[2] << endl;
   
    // how much time is spent reading the sensors?
    static unsigned long sensorTime = 0;
    sensorTime = (9*sensorTime + 1*elapsedTime())/10; // running average
    
    // send
    boolean haveSent = false;
    while( ! haveSent ) {
      Serial << F("Sending...") << endl;
      haveSent = N.sendMessage(transToNodeID) || N.state==M_PROGRAM;
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

    // no need to bootstrap
    bootstrap = false;
    
    digitalWrite(LED, LOW);
  }

}


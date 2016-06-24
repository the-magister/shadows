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

//#define SPOOF_LOCATION 0
//#define SPOOF_CIRCLE 0

#define CALIBRATION_INTERVAL 60000UL // calibrate every minute if there's nothing going on

#define RESEND_INTERVAL 1000UL

systemState lastState;

void setup() {
  Serial.begin(115200);

  // start the radio
  N.begin();

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  // start the range finder
  L.begin(N.whoAmI());

}

unsigned long elapsedTime() {
  static unsigned long then = micros();
  unsigned long now = micros();
  unsigned long delta = now - then;
  then = now;
  return( delta );
}

void loop()
{
//  static Metro heartBeat(1000UL);
//  if ( heartBeat.check() ) {
//    Serial << F(".");
//  }

  // update the radio traffic
//  static Metro resendInterval(RESEND_INTERVAL);
  if ( N.update() ) {
    Serial << F("RX: "); N.printMessage();
//    resendInterval.reset();
//    Serial << F("meNext?") << N.meNext() << endl;
  }

  // do I need to update the position information?
  if ( N.meNext() ) {
    digitalWrite(LED, HIGH);

    // if it's good to recalibrate (nothing sensed), do so.
    static Metro calibrationInterval(CALIBRATION_INTERVAL);
    if ( N.msg.d[0] < IN_PLANE || N.msg.d[1] < IN_PLANE || N.msg.d[2] < IN_PLANE ) {
      // something was detected
      calibrationInterval.reset();
    }
    if( calibrationInterval.check() || N.getState() == M_CALIBRATE ) {
      L.calibrateDistance();
      N.setState(M_NORMAL);
    }

    // how much time is spent by the other nodes?
    static unsigned long otherTime = 0;
    otherTime = (9*otherTime + 1*elapsedTime())/10; // running average
    
    // update distance
    L.readDistance( N.msg );
    // how much time is spent reading the sensors?
    static unsigned long sensorTime = 0;
    sensorTime = (9*sensorTime + 1*elapsedTime())/10; // running average
    
    // calculate positions
    L.calculatePosition( N.msg );
    // how much time is spent by calculating positions?
    static unsigned long locationTime = 0;
    locationTime = (9*locationTime + 1*elapsedTime())/10; // running average

    // send
    N.send();
    // how much time is spent by sending data?
    static unsigned long sendTime = 0;
    sendTime = (9*sendTime + 1*elapsedTime())/10; // running average
 
    // show
    Serial << F("TX: "); N.printMessage();

    digitalWrite(LED, LOW);

    static byte loopCount = 0;
    loopCount++;
    if( loopCount >= 10 ) {
      loopCount = 0;
      Serial << F("=== Timers:") << endl;
      Serial << F("otherTime (us)=") << otherTime << endl;
      Serial << F("sensorTime (us)=") << sensorTime << endl;
      Serial << F("locationTime (us)=") << locationTime << endl;
      Serial << F("sendTime (us)=") << sendTime << endl;
      Serial << F("SUM (sensor+loc+2*send) (us)=") << (sensorTime+locationTime+2*sendTime) << endl;
      Serial << F("===") << endl;
    }

    // record that we sent something
//    resendInterval.reset();
  }

  // do I need to resend position information?
//  if ( N.meLast() && resendInterval.check() && N.getState() != M_PROGRAM) {
//    digitalWrite(LED, HIGH);
//    N.send();
    // show
//    Serial << F("RESEND: "); N.printMessage();
//    digitalWrite(LED, LOW);
//  }  

}
/*
void spoofLocation(Message &msg) {
  // pick a random x
  int x = random(1, BASE_LEN);

  // determine yrange
  const float angle = tan(PI / 3.0);
  int ymax = x <= BASE_LEN / 2 ? angle * (float)x : angle * (float)(BASE_LEN - x);
  int y = random(1, ymax);

  Serial << F("Spoof location.  x=") << x << F(" y=") << y << endl;

  msg.d[0] = round(pow( pow(x, 2.0) + pow(y, 2.0) , 0.5));
  msg.d[2] = round(pow( pow(BASE_LEN - x, 2.0) + pow(y, 2.0) , 0.5));
  msg.d[1] = round(pow( pow(BASE_LEN / 2.0 - x, 2.0) + pow(HEIGHT_LEN - y, 2.0) , 0.5));

  N.printMessage();

  Serial << F("Spoof circle. backcheck:");
  word range, inter;
  //  L.heavyLift(msg.d[0], msg.d[2], msg.d[1], inter, range);
  L.simpleLift(msg.d[0], msg.d[2], inter, range);

}

void spoofCircle(Message & msg) {
  const float r = HEIGHT_CEN - 1.0;
  const float x0 = HALF_BASE;
  const float y0 = HEIGHT_CEN;

  Serial << F("Spoof circle. r=") << r << F(" x0=") << x0 << F(" y0=") << y0 << endl;

  float where = (float)(millis() % 10000UL) / 10000.0 * 2.0 * PI;

  float x = x0 + r * cos(where);
  float y = y0 + r * sin(where);

  Serial << F("Spoof circle. where=") << where << F(" x=") << x << F(" y=") << y << endl;

  msg.d[0] = round(pow( pow(x, 2.0) + pow(y, 2.0) , 0.5));
  msg.d[2] = round(pow( pow(BASE_LEN - x, 2.0) + pow(y, 2.0) , 0.5));
  msg.d[1] = round(pow( pow(BASE_LEN / 2.0 - x, 2.0) + pow(HEIGHT_LEN - y, 2.0) , 0.5));

  N.printMessage();

  Serial << F("Spoof circle. backcheck:");
  word range, inter;
  //  L.heavyLift(msg.d[0], msg.d[2], msg.d[1], inter, range);
  L.simpleLift(msg.d[0], msg.d[2], inter, range);

}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

*/

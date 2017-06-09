// Compile for MoteinoMEGA
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

// network and location information for LightSticks
#include <Network.h>
Network Net;
Distances Dist;

// ultrasonic range finders
#include "Location.h"
Location Loc;

// sound output
#include "Sound.h"
SoftwareSerial wavSerial(WAVE_TX, WAVE_RX);
Sound Sound;

// corner and debug lighting
#include "Lights.h"
Lights Light;

#define DEBUG_UPDATE 0
#define DEBUG_DISTANCE 1
#define DEBUG_ALTITUDE 0
#define DEBUG_COLLINEAR 0
#define DEBUG_AREA 0
#define DEBUG_INTERVAL 0

void setup() {
  Serial.begin(115200);

  Serial << F("Startup.") << endl;

  delay(500);
  
  // start the radio
  Net.begin(&Dist, 255, GROUPID, RF69_915MHZ, 15); // Higher power setting goofing analogRead()!!

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) Net.update();

  // start the range finder
  Loc.begin(&Dist);

  // start the sound
  Sound.begin(&Dist, &wavSerial);

  // start the corner lights
  Light.begin(&Dist);

  Serial << F("Startup complete.") << endl;
}

void loop() {
  // don't broadcast packets if OTA programming is occuring
  static Metro backToNormalAfterProgram(5000UL);
  // update the radio traffic
  boolean haveTraffic = Net.update();
  if ( haveTraffic ) {
    Serial << F("RECV: "); Net.showNetwork();

    if ( Net.state == M_PROGRAM ) {
      // OTA programming is occuring, so reset send lockout timer
      backToNormalAfterProgram.reset();
    } else if ( Net.state == M_CALIBRATE ) {
      // reboot the sensors
      Loc.begin(&Dist);
      Net.state = M_NORMAL;
    }
  }
  // we don't want to get stuck in programming mode if we miss the reset to M_NORMAL, so have a timer tick down
  if ( Net.state != M_NORMAL && backToNormalAfterProgram.check() ) {
    // just don't want to be locked out forever
    Net.state = M_NORMAL;
  }

  // @ KiwiBurn noted need for periodic reboot to the lights
  static Metro rebootEvery(30UL * 1000UL);
  if ( objectInPlane() ) rebootEvery.reset();
  if ( rebootEvery.check() ) {
    rebootEvery.reset();
    Net.state = M_REBOOT;
    Serial << F("Reboot timer expireDist.  Rebooting.") << endl;
    for (byte i = 0; i < 10; i++) {
      Net.sendState();
      delay(5);
    }
    //    resetUsingWatchdog(true);
  }

  /*
     The range finders rely on precision timing as does the radio (via interrupts).
     The ranging is blocking, but the radio work isn't.
     So, we need to take care that the ranging and radio work do not coincide.

     1. get NEW ranging data
     2. send LAST distance packet
     3. calculate the NEW distance packet
     4. update the corner lights and sound
  */

  // make sure the distance packet is sent
  const unsigned long sendTime = ceil((float)(sizeof(Distances) * 8) / 55.55555);
  static Metro sendLockout(sendTime);
  while ( !sendLockout.check());

  // average the sensor readings
  Loc.update();

  // send
  if( Net.state == M_NORMAL ) {
    Net.sendMessage();
    sendLockout.reset();
  } else {
    Serial << "Skipping send.  Network state=" << Net.state << endl;
  }

  // update position information
  Loc.calculateLocation();

  // update sound
  Sound.update();

  // update lights
  Light.update(Net.state);

  if ( DEBUG_UPDATE ) {
    for ( byte i = 0; i < N_RANGE; i++ ) {
      Serial << F("S") << i << F(" reading=") << Loc.currRange[i] << F("\t");
    }
    Serial << endl;
  }

  if ( DEBUG_DISTANCE ) {
    for ( byte i = 0; i < N_RANGE; i++ ) {
      Serial << F("S") << i << F(" range=") << Dist.D[i] << F("\t");
    }
    Serial << endl;
  }

  if ( DEBUG_ALTITUDE ) {
    for ( byte i = 0; i < N_NODES; i++ ) {
      Serial << F("N") << i << F(" Ab=") << Dist.Ab[i] << F("\t") << F("Ah=") << Dist.Ah[i] << F("\t\t");
    }
    Serial << endl;
  }

  if ( DEBUG_COLLINEAR ) {
    for ( byte i = 0; i < N_NODES; i++ ) {
      Serial << F("N") << i << F(" Cb=") << Dist.Cb[i] << F("\t") << F("Ch=") << Dist.Ch[i] << F("\t\t");
    }
    Serial << endl;
  }

  if ( DEBUG_AREA ) {
    for ( byte i = 0; i < N_NODES; i++ ) {
      Serial << F("N") << i << F(" Area=") << Dist.Area[i] << F("\t\t");
    }
    Serial << endl;
  }

  static unsigned long tic = millis();
  static unsigned long toc = millis();
  if ( DEBUG_INTERVAL ) {
    tic = toc;
    toc = millis();
    Serial << F("Update interval (ms)=") << toc - tic << endl;
  }

}

boolean objectInPlane() {

  const word distInPlane = 0.7 * (float)HL;

  byte inPlane0 = Dist.D[0] <= distInPlane;
  byte inPlane1 = Dist.D[1] <= distInPlane;
  byte inPlane2 = Dist.D[2] <= distInPlane;

  return (
           (inPlane0 + inPlane1 + inPlane2) >= 2
         );
}


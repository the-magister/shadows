#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>      
#include <avr/wdt.h>
#include <WirelessHEX69.h> 
#include <EEPROM.h>

#include <FastLED.h>

#include "Animations.h"
#include "Identity.h"
#include "Location.h"
#include "Network.h"

#define LOWACT 15 // little bit of flare
#define HIGHACT 100 // lots of flare when triggered
#define FLAREDUR 5000UL // flare for this long when triggered

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);
   
  // start the radio
  N.begin();

  // startup animation
  A.begin();
  
  // see Animations.h for other options.
  A.startAnimation(A_FIRE);
  A.setActivity(LOWACT);

}

void loop()
{
  // update the radio traffic
  N.update();

  // run it
  A.runAnimation();
  
}


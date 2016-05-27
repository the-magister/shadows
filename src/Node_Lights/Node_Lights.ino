#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>      
#include <avr/wdt.h>
#include <WirelessHEX69.h> 
#include <EEPROM.h>

#include <Network.h>

#include <FastLED.h>

#include "Animations.h"

void setup() {
  Serial.begin(115200);

  // start the radio
  N.begin();

  // startup animation
  A.begin();
  
  // see Animations.h for other options.
  A.startAnimation(A_FIRE);
}

void loop()
{
  // track an idle timer
  static Metro goIdleTimer(10000UL);

  // update the radio traffic
  if( N.update() && N.isObject() ) {
    goIdleTimer.reset(); // reset timer
    A.startAnimation(A_SHADOW, false); // start shadow animation if we've not already

    // pull my x, y and distance information
    long x = N.msg.x[N.whoAmI()-20];
    long y = N.msg.y[N.whoAmI()-20];
    long d = N.msg.d[N.whoAmI()-20];

    // project the path of the object from the opposing sensor to my lights
    long projX = x*HEIGHT_LEN/(HEIGHT_LEN-y); // tangent equivalence
    
    // and the distance from the projected object to my lights
    long projD = d*(HEIGHT_LEN/(HEIGHT_LEN-y)-1); // cosine equivalence

    // map the projected x location to a pixel
    byte projPixel = map(projX, -BASE_LEN/2, BASE_LEN/2, 0, NUM_LEDS-1);

    // map the distance to a coverage of pixels
    byte projArea = map(projD, 0, d+projD, 1, NUM_LEDS/2);

    A.setShadowCenter(projPixel);
    A.setShadowExtent(projArea);
  }

  if( goIdleTimer.check() ) {
    A.startAnimation(A_FIRE, false);
  }
  
  // run it
  A.runAnimation();  
}


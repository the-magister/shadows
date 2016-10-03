#include <Streaming.h>
#include <Metro.h>
#include <FastLED.h>

#define NUM_LEDS 9
#define PIN_CLK 6 // yellow wire on LED strip
#define PIN_DATA 7 // green wire on LED strip
CRGB leds[NUM_LEDS];
#define MASTER_BRIGHTNESS 255


void setup() {
  // put your setup code here, to run once:
  // set up the LEDs
  FastLED.addLeds<WS2801, PIN_DATA, PIN_CLK, BGR>(leds, NUM_LEDS);

  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

}

void loop() {
  // put your main code here, to run repeatedly:
  // Turn the LED on, then pause
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);
  // Now turn the LED off, then pause
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);

}

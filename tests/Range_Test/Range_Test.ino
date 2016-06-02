#include <Streaming.h>

#define PIN_GND 2   // DC return
#define PIN_5V  3   // Vcc in range 3-5.5VDC
#define PIN_RX  5   // hold high/open for ranging. hold low to stop ranging.
#define PIN_PW  7   // pulse width representation with scale factor of 140 uS per inch

void setup() {
  Serial.begin(115200);
  
  // put your setup code here, to run once:
  Serial << F("Startup.") << endl;

  digitalWrite(PIN_RX, LOW); pinMode(PIN_RX, OUTPUT);
  digitalWrite(PIN_5V, LOW); pinMode(PIN_5V, OUTPUT);
  digitalWrite(PIN_GND, LOW); pinMode(PIN_GND, OUTPUT);

  // calibrate
  
  // depower the MaxSonar
  digitalWrite(PIN_5V, LOW);
  // set the range pin to high
  digitalWrite(PIN_RX, HIGH);

  // wait for depower
  delay(1000);

  // power the MaxSonar
  digitalWrite(PIN_5V, HIGH);

  // wait for calibration; should take about 250 ms
  delay(1000);

  // set the range pin to low
  digitalWrite(PIN_RX, LOW);  

  // wait for another cycle, since we'll likely take a reading immediately hereafter
  delay(50);

  Serial << F("calibrated range finder.") << endl;


}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(PIN_RX, HIGH); // range
  unsigned long pulseTime = pulseIn(PIN_PW, HIGH);
  digitalWrite(PIN_RX, LOW); // stop ranging

  Serial << F("pulseTime=") << pulseTime << endl;
  
  // convert the number of us to decainches.  e.g. 1200 decainches is 12 inches.
  float decaInches = ( pulseTime * 100.0 ) / 147.0; // max 25510.

  Serial << F("range=") << decaInches << endl;

  delay(1000);

}

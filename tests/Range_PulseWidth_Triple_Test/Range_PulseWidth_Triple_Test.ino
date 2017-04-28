// add the following to Preferences->Board Manager URL
// Add Moteino to Board Manager
// Compile for Moteino Mega

/*
    pinout: https://www.flickr.com/photos/15304611@N03/14677033948

    LED: D15
    Transceiver: D2, D4-7
    Flash: D5-7, 23
*/
#define LED 15

#include <Streaming.h>
#include <Metro.h>
#include <SoftwareSerial.h>

#define C0_PW 19      // wire to Sonar PW via shifter
#define C0_RNG 20     // wire to Sonar RX via shifter
#define C0_EN 21      // wire to Sonar +5 via shifter
#define C0_DATA 22    // wire to lighting DATA IN via shifter

#define C1_PW 14      //  wire to Sonar PW via shifter
// skipping D15; has an LED on that pin.
#define C1_RNG 16     // wire to Sonar RX via shifter
#define C1_EN 17     // wire to Sonar PW via shifter
#define C1_DATA 18    // wire to lighting DATA IN via shifter

#define C2_PW 10      // wire to Sonar PW via shifter
#define C2_RNG 11     // wire to Sonar RX via shifter
#define C2_EN 12      // wire to Sonar +5 via shifter
#define C2_DATA 13    // wire to lighting DATA IN via shifter

void setup() {
  // LED
  pinMode(LED, OUTPUT);

  Serial.begin(115200);

  Serial << "Startup" << endl;

  boot(C0_EN, C0_RNG, C0_PW);
  boot(C1_EN, C1_RNG, C1_PW);
  boot(C2_EN, C2_RNG, C2_PW);
}

void boot(byte EN_PIN, byte RNG_PIN, byte PW_PIN) {

  Serial << "Booting sensor." << endl;

  // for each sensor
  digitalWrite(RNG_PIN, LOW);
  digitalWrite(EN_PIN, LOW);
  pinMode(RNG_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(PW_PIN, INPUT);

  // wait for depower
  delay(100);

  // calibrate
  digitalWrite(EN_PIN, HIGH);
  digitalWrite(RNG_PIN, HIGH);
  delay(250);

  // turn it off
  digitalWrite(RNG_PIN, LOW);
  delay(50);

}

int range(byte RNG_PIN, byte PW_PIN) {
  digitalWrite(LED, HIGH);
  digitalWrite(RNG_PIN, HIGH);
  unsigned long val = pulseIn(PW_PIN, HIGH);

  digitalWrite(RNG_PIN, LOW);
  digitalWrite(LED, LOW);

  return ( (int)(val / 147UL) );
}

void loop() {

  static int r0 = 0;
  static int r1 = 0;
  static int r2 = 0;
  static unsigned long tic = millis();
  static unsigned long toc = millis();

  r0 = range(C0_RNG, C0_PW);
  r1 = range(C1_RNG, C1_PW);
  r2 = range(C2_RNG, C2_PW);

  toc = millis();
  Serial << toc - tic << "\t" << r0 << "\t" << r1 << "\t" << r2 << endl;
  tic = toc;



}


// add the following to Preferences->Board Manager URL
//    https://lowpowerlab.github.io/MoteinoCore/package_LowPowerLab_index.json
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

// 22,21,20,19
#define C0_PW   21    // wire to Sonar PW via shifter
#define C0_RNG  20    // wire to Sonar RX via shifter
#define C0_EN   22    // wire to Sonar +5 via shifter
#define C0_DATA 19    // wire to lighting DATA IN via shifter

// 18,17,16,14
// skipping D15; has an LED on that pin.
#define C1_PW   17    // wire to Sonar PW via shifter
#define C1_RNG  16    // wire to Sonar RX via shifter
#define C1_EN   18    // wire to Sonar +5 via shifter
#define C1_DATA 14    // wire to lighting DATA IN via shifter

// 13,12,11,10
#define C2_PW   12    // wire to Sonar PW via shifter
#define C2_RNG  11    // wire to Sonar RX via shifter
#define C2_EN   13    // wire to Sonar +5 via shifter
#define C2_DATA 10    // wire to lighting DATA IN via shifter

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
  // I'm going to set the PW pin LOW, too, in case that's keeping the board powered
  digitalWrite(PW_PIN, LOW);
  pinMode(RNG_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(PW_PIN, OUTPUT);

  // wait for depower
  delay(500);

  // calibrate
  digitalWrite(EN_PIN, HIGH);
  digitalWrite(RNG_PIN, HIGH);
  delay(300);

  // turn it off
  digitalWrite(RNG_PIN, LOW);
  delay(100);

  // now, flip the PW pin back to INPUT
  pinMode(PW_PIN, INPUT);
}

word range(byte RNG_PIN, byte PW_PIN) {
  digitalWrite(LED, HIGH);
  digitalWrite(RNG_PIN, HIGH);
  unsigned long val = pulseIn(PW_PIN, HIGH);

  digitalWrite(RNG_PIN, LOW);
  digitalWrite(LED, LOW);

//  return ( (int)(val / 147UL) );
//  return ( (int)(val / 14UL) );
  return ( (word)constrain(val, 0UL, 65535UL) );
}

void loop() {

  static word r0 = 0;
  static word r1 = 0;
  static word r2 = 0;
  static unsigned long tic = millis();
  static unsigned long toc = millis();

  static unsigned long dtime = 0;

  r0 = range(C0_RNG, C0_PW);
//  delayMicroseconds(dtime);
  r1 = range(C1_RNG, C1_PW);
//  delayMicroseconds(dtime);
  r2 = range(C2_RNG, C2_PW);
//  delayMicroseconds(dtime);

//  if( r0<10000 || r1<10000 || r2<10000 ) {
//      dtime+=100;
//  }

  toc = millis();
  Serial << dtime << "\t" << toc - tic << "\t" << r0 << "\t" << r1 << "\t" << r2 << endl;
//  Serial << r0 << "\t" << r1 << "\t" << r2 << endl;
  tic = toc;



}


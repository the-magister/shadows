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

#define N_RANGE        3 // 

const byte PIN_PW[] = {21, 17, 12};
const byte PIN_RNG[] = {20, 16, 11};
const byte PIN_EN[] = {22, 18, 13};
const byte PIN_DATA[] = {19, 14, 10};

unsigned long currRange[] = {0, 0, 0}; // value of last range
unsigned long avgRange[] = {0, 0, 0}; // smoothed ranges

const word HL = 10600;  // altitude of the triangle; sqrt(3)/2 * SL

/*
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
*/

void setup() {
  // LED
  pinMode(LED, OUTPUT);

  Serial.begin(115200);

  Serial << "Startup" << endl;

  bootAll();
  
}

void boot(byte sIndex, byte EN_PIN, byte RNG_PIN, byte PW_PIN) {

  Serial << "Location.  Booting sensor " << sIndex << endl;

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

void bootAll() {
  for( byte i=0; i<N_RANGE; i++ ) boot(i, PIN_EN[i], PIN_RNG[i], PIN_PW[i]);
}

word range(byte RNG_PIN, byte PW_PIN) {

  digitalWrite(RNG_PIN, HIGH);
  unsigned long val = pulseIn(PW_PIN, HIGH);
  digitalWrite(RNG_PIN, LOW);

  // constrained to HL
  return ( (word)constrain(val, 0, HL) );
}

void rangeAll(byte smoothing=3) {
  for( byte i=0; i<N_RANGE; i++ ) {
    currRange[i] = range(PIN_RNG[i], PIN_PW[i]);
    avgRange[i] = (currRange[i] + avgRange[i]*smoothing)/(1+smoothing); 
  }
}

void loop() {

  static unsigned long tic, toc;
  
  rangeAll();

  toc = millis();

  for( byte i=0; i<N_RANGE; i++ ) Serial << currRange[i] << "\t" << avgRange[i] << "\t";
  Serial << endl;
  
  tic = toc;

}


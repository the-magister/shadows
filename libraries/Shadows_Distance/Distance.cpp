#include "Distance.h"

void Distance::begin() {
  Serial << F("Distance. startup.") << endl;

  digitalWrite(PIN_GND, LOW); pinMode(PIN_GND, OUTPUT);
  pinMode(PIN_PW, INPUT);

  digitalWrite(PIN_RX, LOW); pinMode(PIN_RX, OUTPUT);

  digitalWrite(PIN_VCC, HIGH); pinMode(PIN_VCC, OUTPUT);

  this->calibrated = false;

  Serial << F("Distance. startup complete.") << endl;
}

word Distance::read() {
  if( !this->calibrated ) calibrate();

  // record the PW length
  digitalWrite(PIN_RX, HIGH); // range
  unsigned long pulseTime = pulseIn(PIN_PW, HIGH);
  digitalWrite(PIN_RX, LOW); // stop ranging

  // convert to distance in centainches; 1 inch = 10 centainches
  unsigned long centaInches = ( pulseTime * 10 ) / 147; 

  return( centaInches );
}

void Distance::calibrate() {
  Serial << F("Location.  calibrating range finder...") << endl;

  // depower the MaxSonar
  digitalWrite(PIN_VCC, LOW);
  // set the range pin to high
  digitalWrite(PIN_RX, HIGH);

  // wait for depower
  delay(250);

  // power the MaxSonar
  digitalWrite(PIN_VCC, HIGH);

  // wait for calibration; should take about 250 ms
  delay(250);

  // set the range pin to low
  digitalWrite(PIN_RX, LOW);

  // wait for another cycle, since we'll likely take a reading immediately hereafter
  delay(50);

  this->calibrated = true;

  Serial << F("Location.  calibrated range finder.") << endl;
}

Distance D;


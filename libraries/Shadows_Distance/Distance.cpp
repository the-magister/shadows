#include "Distance.h"

void Distance::begin() {
  Serial << F("Distance.  startup.") << endl;

  // set the pin state before going ahead

  // stop ranging
  digitalWrite(PIN_RX, LOW);
  pinMode(PIN_RX, OUTPUT);

  // provide ground path
  digitalWrite(PIN_GND, LOW); 
  pinMode(PIN_GND, OUTPUT);

  // power the transceiver
  pinMode(PIN_VCC, OUTPUT);
  digitalWrite(PIN_VCC, HIGH);

  // input for range/distance
  pinMode(PIN_PW, INPUT);

  Serial << F("Distance.  startup complete.") << endl;
}

void Distance::stop() {
  Serial << F("Distance.  depowering range finder.") << endl;
  // depower
  digitalWrite(PIN_VCC, LOW);
  // stop ranging (could also power the transceiver?  dunno.)
  digitalWrite(PIN_RX, LOW); // stop ranging

  // wait for depower
  delay(1000UL);
}


word Distance::read() {
  // power the transceiver (if it's not already powered)
  digitalWrite(PIN_VCC, HIGH);

  // record the PW length
  digitalWrite(PIN_RX, HIGH); // range
  unsigned long pulseTime = pulseIn(PIN_PW, HIGH);
  digitalWrite(PIN_RX, LOW); // stop ranging

  // convert to distance in centainches; 1 inch = 10 centainches
  unsigned long centaInches = ( pulseTime * 10 ) / 147; 

  return( centaInches );
}


Distance D;


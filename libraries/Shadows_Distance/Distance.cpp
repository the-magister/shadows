#include "Distance.h"

void Distance::begin() {
  Serial << F("Distance.  startup....") << endl;

  // set the pin state before going ahead
  Serial << F("Distance.  depowering range finder.") << endl;
  // depower
  digitalWrite(PIN_VCC, LOW);
  // stop ranging (could also power the transceiver?  dunno.)
  digitalWrite(PIN_RX, LOW); // stop ranging
  // provide ground path
  digitalWrite(PIN_GND, LOW); 
  
  // pin modes
  pinMode(PIN_RX, OUTPUT);
  pinMode(PIN_GND, OUTPUT);
  pinMode(PIN_VCC, OUTPUT);
  // input for range/distance
  pinMode(PIN_PW, INPUT);

  // wait for depower
  delay(200);
  
  // power the transceiver (if it's not already powered)
  digitalWrite(PIN_VCC, HIGH);

  // run the first (calibration) read
  word dist = this->read();
  
  Serial << F("Distance.  calibration reading(cin)=") << dist << endl;
  
  Serial << F("Distance.  startup complete.") << endl;
}

word Distance::read() {

  // record the PW length
  digitalWrite(PIN_RX, HIGH); // range
  unsigned long pulseTime = pulseIn(PIN_PW, HIGH);
  digitalWrite(PIN_RX, LOW); // stop ranging

  // convert to distance in centainches; 1 inch = 10 centainches
  unsigned long centaInches = ( pulseTime * 10 ) / 147; 

  return( centaInches );
}


Distance D;


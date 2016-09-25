#include "Distance.h"

void Distance::begin() {
  Serial << F("Distance.  startup....") << endl;

  digitalWrite(PIN_START_RANGE, LOW);
  pinMode(PIN_START_RANGE, OUTPUT);

//  Timing Description
// 250mS after power-up, the LV-MaxSonar-EZ is ready to accept the RX command. 
  delay(300); // delay after power up

  // AN Output Constantly Looping:
  // "To start the continuous loop, bring the RX pin high for a time greater than 20us but 
  // less than 48ms and return to ground."
  
  digitalWrite(PIN_START_RANGE, HIGH);
  delay(5);
  digitalWrite(PIN_START_RANGE, LOW);
  
  Serial << F("Distance.  startup complete.") << endl;
}


boolean Distance::update() {

  word reading[N_RANGE];
  reading[0] = analogRead(PIN_RANGE_1);
  reading[1] = analogRead(PIN_RANGE_2);
  reading[2] = analogRead(PIN_RANGE_3);
  
  // AN Output:
  // Outputs analog voltage with a scaling factor of (Vcc/512) per inch. 
  // A supply of 5V yields ~9.8mV/in. and 3.3V yields ~6.4mV/in. 
  // The output is buffered and corresponds to the most recent range data.
  
  // Arduino analogRead:
  // This means that it will map input voltages between 0 and 5 volts into integer 
  // values between 0 and 1023. This yields a resolution between readings of: 
  // 5 volts / 1024 units or, .0049 volts (4.9 mV) per unit.
  
  // To Mike's reading, this means a unit of "1" in the analog read is 0.5 in.
  // distance [in] = reading [unit] * (5/1024 [volts/unit]) / (5/512 [volts/in])
  // distance [in] = reading [unit] * (1/2 [in/unit])
  
  // assume the readings haven't changed
  boolean newReading = false;
  
  for( byte i=0; i<N_RANGE; i++ ) {
    reading[i] *= 5; // /2 [=] in, *5 [=] centainches
  
    if( distance[i] != reading[i] ) {
      newReading = true;
    }
	
    distance[i] = reading[i];
	
  }
  
  return( newReading );
}


Distance D;


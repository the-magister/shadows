#include <Streaming.h>

#include <Distance.h>

void setup() {
  Serial.begin(115200);

  // start the range finder
  D.begin();
}

void loop() {

  if( D.update() ) {
      for( byte i=0; i<N_RANGE; i++ ) {
        Serial << F("Sensor=") << i << F("\trange(cin)=") << D.distance[i];
      }
      Serial << endl;
      delay(100); // stop spam
  }
  
}

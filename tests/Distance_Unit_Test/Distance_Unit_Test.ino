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
        Serial << F("S") << i << F(" range(cin)=") << D.distance[i] << F("\t");
      }
      Serial << endl;

      delay(100); // stop spam
  }
      
}

// readings 212

// Analog, (Vcc/512) / inch

// 212/1023 
// reading : 0.2 V

#include "Sound.h"

void Sound::begin(Distances *D, SoftwareSerial *S) {

  wavSerial = S;
  
  wavSerial->begin(57600);
  if ( !wavSerial ) {
    Serial << F("Error setting up WAV board serial.") << endl;
    while(1);
  } else {
    Serial << F("Set up WAV board serial.") << endl;
  }
  
  wav.start(wavSerial);

  // name the tracks with these prefaces
  track[0] = 100;
  track[1] = 101;
  track[2] = 102;

  masterGain = 0;
  trackGain = -3;

  wav.masterGain(masterGain);
  wav.stopAllTracks();

  wav.trackPlaySolo(1);
  wav.trackGain(1, masterGain);
  delay(1000);
  wav.trackStop(1);
  
  for( byte i=0; i<N_RANGE; i++ ) {
    wav.trackPlayPoly(track[i]);
    wav.trackGain(track[i], -100);
    wav.trackLoop(track[i], true);
  }

  Serial << F("Set up Sound.") << endl;

  this->d = D;
}
void Sound::update() {
  const float power = 1.5;
  const unsigned long scale = pow(HL, power);
  
  for( byte i=0; i<N_RANGE; i++ ) {
    int mapping = map((unsigned long)pow(d->D[i],power), 0UL, scale, trackGain, trackGain-30);
    wav.trackGain(track[i], mapping);

//    Serial << F("t=") << i << F(" d=") << d->D[i] << F(" m=") << mapping << F("\t");
  }
//  Serial << endl;
}


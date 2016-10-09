#include <Streaming.h>
#include <Metro.h>
#include <wavTrigger.h>
#include <SoftwareSerial.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

// Shadows specific libraries.
#include <Network.h>
Distances D;

#define WAVE_RX 7 // Connect pin 7 to WAV Trigger RX
#define WAVE_TX 6 // Connect pin 6 to WAV Trigger TX
// make sure they share a Ground plane
SoftwareSerial wavSerial(WAVE_TX, WAVE_RX); 

class Sound {
  public:
    void begin(Distances *D, SoftwareSerial *S);
    void update();
  private:
    wavTrigger Sound;
    SoftwareSerial *wavSerial; // (6, 7);
    
    int track[N_RANGE]; //  = {100, 101, 102);
    int masterGain; //  = 0;
    int trackGain; // = -3;
    
    Distances *d;
};
void Sound::begin(Distances *D, SoftwareSerial *S) {

  wavSerial = S;
  
  wavSerial->begin(57600);
  if ( !wavSerial ) {
    Serial << F("Error setting up WAV board serial.") << endl;
    while(1);
  } else {
    Serial << F("Set up WAV board serial.") << endl;
  }
  
  Sound.start(wavSerial);

  // name the tracks with these prefaces
  track[0] = 100;
  track[1] = 101;
  track[2] = 102;

  masterGain = 0;
  trackGain = -3;

  Sound.masterGain(masterGain);
  Sound.stopAllTracks();
  for( byte i=0; i<N_RANGE; i++ ) {
    Sound.trackPlayPoly(track[i]);
    Sound.trackGain(track[i], -100);
    Sound.trackLoop(track[i], true);
  }

  Serial << F("Set up Sound") << endl;

  this->d = D;
}
void Sound::update() {
  for( byte i=0; i<N_RANGE; i++ ) {
    int distance = constrain(d->D[i], 0, HL);
    int mapping = map(distance, 0, HL, trackGain, trackGain-30);
    Sound.trackGain(track[i], mapping);

    Serial << F("t=") << i << F(" d=") << distance << F(" m=") << mapping << F("\t");
  }
}

Sound sound;

void setup() {
  Serial.begin(115200);

  sound.begin(&D, &wavSerial);
  
  // set some distances
  randomSeed(analogRead(0));
  D.D[0] = random(0,HL);
  D.D[1] = random(0,HL);
  D.D[2] = random(0,HL);

  Serial << F("Startup complete.") << endl;
}

void loop() {

  static int dir[N_RANGE]={-3,2,-5};
  
  for( byte i=0; i<N_RANGE; i++ ) {
    if( (int)D.D[i]+dir[i] > HL || (int)D.D[i]+dir[i] < 0 ) dir[i]=-dir[i];
    D.D[i] += dir[i];
  }

  sound.update();
  
  Serial << endl;

  delay(100);
   
}



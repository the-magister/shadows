#ifndef Sound_h
#define Sound_h

#include <Arduino.h>

#include <wavTrigger.h>
#include <SoftwareSerial.h>
#include <Network.h>

#define WAVE_RX 7 // Connect pin 7 to WAV Trigger RX
#define WAVE_TX 6 // Connect pin 6 to WAV Trigger TX
// make sure they share a Ground plane


class Sound {
  public:
    void begin(Distances *D, SoftwareSerial *S);
    void update();
  private:
    wavTrigger wav;
    SoftwareSerial *wavSerial; // (6, 7);
    
    int track[N_RANGE]; //  = {100, 101, 102);
    int masterGain; //  = 0;
    int trackGain; // = -3;
    
    Distances *d;
};

#endif

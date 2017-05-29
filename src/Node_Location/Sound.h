#ifndef Sound_h
#define Sound_h

#include <Arduino.h>

#include <wavTrigger.h>
#include <SoftwareSerial.h>
#include <Network.h>

// Moteino MEGA A6=D30 A7=D31
#define WAVE_RX 30 // Connect A6/D30 to WAV Trigger RX
#define WAVE_TX 31 // Connect A7/D31 to WAV Trigger TX
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

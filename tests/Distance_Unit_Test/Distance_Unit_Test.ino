#include <Streaming.h>

#include <Distance.h>

void setup() {
  Serial.begin(115200);

  // start the range finder
  D.begin();
  D.stop();
}

unsigned long elapsedTime() {
  static unsigned long then = micros();
  unsigned long now = micros();
  unsigned long delta = now - then;
  then = now;
  return ( delta );
}

void loop() {
  // track the time it takes for everything.
  unsigned long tic = elapsedTime();

  // calibrate at the start and periodically
  static byte loopCount = 0;
  loopCount++;
  if ( loopCount >= 60 ) {
    loopCount = 0;
    tic = elapsedTime();
    D.stop();
    tic = elapsedTime();
    Serial << F("D.stop() time (us)=") << tic << endl;
  }

  // update distance
  tic = elapsedTime();
  word dist = D.read();
  tic = elapsedTime();

  Serial << F("range(cin)=") << dist;
  Serial << F("\trange(in)=") << (float)dist / 10.0;
  Serial << F("\tD.read() time (us)=") << tic << endl;

  delay(1000);
}

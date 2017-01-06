#include <Streaming.h>
#include <Metro.h>

#define PIN_START_RANGE 5 // trigger for ranging start; read the fps; note this is Dn
#define N_RANGE 3
const byte rangePin[N_RANGE] = { 7, 6, 5 }; // range from sonar 10, 11, 12, respectivel; note this is An

unsigned long rangeValue[N_RANGE] = { 650, 650, 650 };

void begin() {
  // use a 1.1 V internal reference to increase resolution
  analogReference(INTERNAL);
  digitalWrite(A5, LOW);
  digitalWrite(A6, HIGH);
  digitalWrite(A7, HIGH);
  
  // has the effect of stopping any ongoing round-robin
  digitalWrite(PIN_START_RANGE, LOW);
  pinMode(PIN_START_RANGE, OUTPUT);

  // 250 ms after power up
  delay(500);

  // send a start pulse
  digitalWrite(PIN_START_RANGE, HIGH);
  delay(5);
  digitalWrite(PIN_START_RANGE, LOW);

  // flip to high-impediance pin state so as to not clobber the return round-robin inc.  
  pinMode(PIN_START_RANGE, INPUT); 
}

void update() {
// It takes about 100 microseconds (0.0001 s) to read an analog input, so the maximum reading rate is about 10,000 times a second.
  const byte nUp = 50;
  unsigned long pool[N_RANGE] = {0,0,0};
  
  for( byte i=0; i<nUp; i++ ) {
    for ( byte n = 0; n < N_RANGE; n++ ) {
      pool[n] += analogRead(rangePin[n]);
    }
  }

 for ( byte i = 0; i < N_RANGE; i++ ) {
    rangeValue[i] = pool[i] / nUp;
//    d->D[i] = constrain( reading[i], 0, HL);
  }


}

void setup() {
  Serial.begin(250000);

  // fire up the ranging
  begin();
}

void loop() {
  update();

  Serial << F("0,");

  for( byte n=0; n<N_RANGE; n++ )
    Serial << analogRead(rangePin[n]) << F(",");

  Serial << F("1024,");

  for( byte n=0; n<N_RANGE; n++ )
    Serial << rangeValue[n] << F(",");
  

  Serial << endl;

//  delay(10);
}


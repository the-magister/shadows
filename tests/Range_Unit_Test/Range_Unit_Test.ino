#include <Streaming.h>
#include <Metro.h>

#define PIN_START_RANGE 5 // trigger for ranging start; read the fps; note this is Dn
#define N_RANGE 3
const byte rangePin[N_RANGE] = { 7, 5, 6 }; // range from sonar 10, 11, 12, respectivel; note this is An

void stopRange() {
  // has the effect of stopping any ongoing round-robin
  digitalWrite(PIN_START_RANGE, LOW);
  pinMode(PIN_START_RANGE, OUTPUT);
}

void startRange() {
  pinMode(PIN_START_RANGE, OUTPUT);
  digitalWrite(PIN_START_RANGE, HIGH);
  delay(10);
  digitalWrite(PIN_START_RANGE, LOW);

  pinMode(PIN_START_RANGE, INPUT); // flip to high-impediance pin state so as to not clobber the return round-robin inc.
}

void setup() {
  Serial.begin(115200);

  //  Timing Description
  // 250mS after power-up, the LV-MaxSonar-EZ is ready to accept the RX command.
  delay(500); // delay after power up

  // start the range finder
  stopRange();
  delay(500);
  startRange();

  analogReference(INTERNAL);
  
  Serial << F("S0,S1,S2") << endl;
}

void loop() {
  Serial << analogRead(rangePin[0]) << F(","); // Blue; bottom
  Serial << analogRead(rangePin[1]) << F(","); // Red: north upper
  Serial << analogRead(rangePin[2]) << F(","); // Yellow: south upper
  Serial << F("1024") << endl; 
  delay(1);
}

// readings: 212 ie. 60"

// Analog, (Vcc/512) / inch

// 212/1023
// reading : 0.2 V

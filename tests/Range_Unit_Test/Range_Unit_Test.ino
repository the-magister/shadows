#include <ResponsiveAnalogRead.h>
#include <Streaming.h>
#include <Metro.h>

#define PIN_START_RANGE 5 // trigger for ranging start; read the fps; note this is Dn
#define N_RANGE 3
const byte rangePin[N_RANGE] = { 7, 6, 5 }; // range from sonar 10, 11, 12, respectivel; note this is An

ResponsiveAnalogRead analog[N_RANGE] = {
  {rangePin[0], false, 0.005},
  {rangePin[1], false, 0.005},
  {rangePin[2], false, 0.005}
};

void begin() {
  // use a 1.1 V internal reference to increase resolution
  analogReference(INTERNAL);
  
  // has the effect of stopping any ongoing round-robin
  digitalWrite(PIN_START_RANGE, LOW);
  pinMode(PIN_START_RANGE, OUTPUT);

  // 250 ms after power up
  delay(250);

  // send a start pulse
  digitalWrite(PIN_START_RANGE, HIGH);
  delay(5);
  digitalWrite(PIN_START_RANGE, LOW);

  // flip to high-impediance pin state so as to not clobber the return round-robin inc.  
  pinMode(PIN_START_RANGE, INPUT); 
}

void update() {
  // new C++ iterators are hawt.
  for( ResponsiveAnalogRead & s : analog ) s.update();
}

void setup() {
  Serial.begin(250000);

  // fire up the ranging
  begin();
}

void loop() {
  update();

  Serial << F("0,");

  // new C++ iterators are hawt.
//  for( ResponsiveAnalogRead & s : analog ) {
//    Serial << s.getRawValue() << F(",");
//  }

  Serial << F("1024,");

  // new C++ iterators are hawt.
  for( ResponsiveAnalogRead & s : analog ) {
    Serial << s.getValue() << F(",");
  }

  Serial << endl;

//  delay(10);
}


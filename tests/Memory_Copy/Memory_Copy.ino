#include <Streaming.h>

#define LED 13

// this is the animation code that we want to push to the Slaves
class Animation {
  public:
    void setup();
    void update(boolean light);
};
void Animation::setup() {
  pinMode(LED, OUTPUT); 
}
void Animation::update(boolean light) {
  // do something useful with the message
  digitalWrite( LED, light ? HIGH : LOW );  
}

Animation a;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial << F("Startup. complete") << endl;

  a.setup();
  Serial << F("sizeof animation=") << sizeof(a) << endl;
}

void loop() {
  // run the animation straight, as-is.  

  boolean value = random(0, 2);
  a.update(value);

  delay(1000);
}

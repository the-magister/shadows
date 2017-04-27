// add the following to Preferences->Board Manager URL
// Add Moteino to Board Manager
// Compile for Moteino Mega

/* 
 *  pinout: https://www.flickr.com/photos/15304611@N03/14677033948
 *  
 *  LED: D15
 *  Transceiver: D2, D4-7
 *  Flash: D5-7, 23
 */
#define LED 15

#include <Streaming.h>
#include <Metro.h>

#define PW0 16 // connect to PW on sonar
#define RX0 17 // connect to RX on sonar

#define PW1 20 // connect to PW on sonar
#define RX1 21 // connect to RX on sonar

void setup() {
  // LED
  pinMode(LED, OUTPUT);
  
  // for each sensor, set RX low to prevent startup
  pinMode(RX0, OUTPUT);
  pinMode(RX1, OUTPUT);
  digitalWrite(RX0, LOW);
  digitalWrite(RX1, LOW);

  // setup for PW
  pinMode(PW0, INPUT);
  pinMode(PW1, INPUT);
  
  Serial.begin(115200);

  // 250mS after power-up, the LV-MaxSonar-EZ is ready to accept the RX command.
  delay(250); // delay after power up

  Serial << "Startup" << endl;

  Serial << "Startup 0." << endl;
  // calibrate
  digitalWrite(RX0, HIGH);
  delay(250);
  // turn it off
  digitalWrite(RX0, LOW);
  delay(50);
  
  Serial << "Startup 1." << endl;
  // calibrate
  digitalWrite(RX1, HIGH);
  delay(250);
  // turn it off
  digitalWrite(RX1, LOW);
  delay(50);

}

byte range0() {
    digitalWrite(LED, HIGH);
    digitalWrite(RX0, HIGH);
    unsigned long val = pulseIn(PW0, HIGH);
    digitalWrite(RX0, LOW);
    digitalWrite(LED, LOW);

    return( (byte)(val/147UL) );
}

byte range1() {
    digitalWrite(LED, HIGH);
    digitalWrite(RX1, HIGH);
    unsigned long val = pulseIn(PW1, HIGH);
    digitalWrite(RX1, LOW);
    digitalWrite(LED, LOW);

    return( (byte)(val/147UL) );
}

void loop() {

  static int r0 = 0;
  static int r1 = 0;
  static unsigned long tic = millis();
  static unsigned long toc = millis();
  
  r0 = range0();
  r1 = range1(); 

  toc = millis();
  Serial << toc - tic << "\t" << r0 << "\t" << r1 << endl;
  tic = toc;

}


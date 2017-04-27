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
#include <SoftwareSerial.h>

#define TX0 18 // connect to TX on sonar
#define RX0 17 // connect to RX on sonar
SoftwareSerial range0(TX0, RX0, true); // TX, RX on sonar 0.
#define TX1 22 // connect to TX on sonar
#define RX1 21 // connect to RX on sonar
SoftwareSerial range1(TX1, RX1, true); // TX, RX on sonar 1.

void setup() {
  // LED
  pinMode(LED, OUTPUT);
  
  // for each sensor, set RX low to prevent startup
  pinMode(RX0, OUTPUT);
  pinMode(RX1, OUTPUT);
  digitalWrite(RX0, LOW);
  digitalWrite(RX1, LOW);
  
  Serial.begin(115200);

  // 250mS after power-up, the LV-MaxSonar-EZ is ready to accept the RX command.
  delay(250); // delay after power up

  Serial << "Startup" << endl;

  Serial << "Startup 0." << endl;
  // calibrate
  range0.begin(9600);
  digitalWrite(RX0, HIGH);
  delay(250);
  // turn it off
  digitalWrite(RX0, LOW);
  delay(50);

  Serial << "Startup 1." << endl;
  // calibrate
  range1.begin(9600);
  digitalWrite(RX1, HIGH);
  delay(250);
  // turn it off
  digitalWrite(RX1, LOW);
  delay(50);

  // start with range0
  do_range0();
}

void do_range0() {
    digitalWrite(RX0, HIGH);
    delay(1);
    digitalWrite(RX0, LOW);

    digitalWrite(LED, HIGH);
    range0.listen();
}

void do_range1() {
    digitalWrite(RX1, HIGH);
    delay(1);
    digitalWrite(RX1, LOW);

    digitalWrite(LED, LOW);
    range1.listen();
}

void loop() {

  static int r0 = 0;
  static int r1 = 0;
  static unsigned long tic = millis();
  static unsigned long toc = millis();
  static unsigned long elapsed = 0;
  
  if( range0.available() && range0.read() == 'R') {
    delay(5);
    r0 = range0.parseInt();

    do_range1();

    Serial << elapsed << "\t" << r0 << "\t" << r1 << endl;
  }


  if( range1.available() && range1.read() == 'R') {
    delay(5);
    r1 = range1.parseInt(); 

    do_range0();

    toc = millis();
    elapsed = toc - tic;
    tic = toc;

    Serial << elapsed << "\t" << r0 << "\t" << r1 << endl;
}

  
}


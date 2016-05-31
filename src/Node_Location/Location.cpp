#include "Location.h"

void Location::begin(byte myNodeID) {
  Serial << F("Location. startup with rangePin=") << rangePin << F(" and pwPin=") << pwPin << endl;

  pinMode(PW_PIN, INPUT);
  digitalWrite(RANGE_PIN, LOW);
  pinMode(RANGE_PIN, OUTPUT);

  // index accessor
  this->myIndex = N.whoAmI()-10;
  Serial << F("Location.  storing distance information in messsage index=") << this->myIndex << endl;

  this->calibrated = false;
}

void Location::readDistance(Message &msg) {
  // do we need the initial calibration?
  if( !this->calibrated ) {
    this->calibrated = true;
    digitalWrite(RANGE_PIN, HIGH); // drive high, should take about 250 ms
    delay(350);
    digitalWrite(RANGE_PIN, LOW);
  }

  digitalWrite(RANGE_PIN, HIGH); // range
  float inches = pulseIn(PW_PIN, HIGH, 37500UL) / 147.0;
  digitalWrite(RANGE_PIN, LOW); // stop ranging

  Serial << F("Location. range=") << inches << endl;
  msg.d[this->myIndex] = inches>255 ? 255 : round(inches);
}

void Location::calculatePosition(Message &msg) {
  // I'm going to avoid using sine and cosine, as those are heavy on a non-FPU machine
  float A, y, x, inter, range;

  Serial << F("Location. calculatePosition A20: ");
  heavyLift(msg.d[2], msg.d[1], msg.d[0], msg.inter[0], msg.range[0]);

  Serial << F("Location. calculatePosition A21: ");
  heavyLift(msg.d[0], msg.d[2], msg.d[1], msg.inter[1], msg.range[1]);

  Serial << F("Location. calculatePosition A22: ");
  heavyLift(msg.d[1], msg.d[0], msg.d[2], msg.inter[2], msg.range[2]);

}

float Location::areaFromDistances(float lA, float lB, float lC) {
  float s = (lA + lB + lC) / 2.0;
  Serial << F(" sp=") << s;
  return ( pow( s * (s - lA) * (s - lB) * (s - lC), 0.5) );
}

float Location::yFromArea(float area, float base) {
  return ( 2.0 * area / base ); // base=this->length
}

float Location::xFromHeight(float y, float l) {
  return ( pow( pow(l, 2.0) - pow(y, 2.0), 0.5 ) ); // l is distance from vertex
}

void Location::heavyLift(byte leftRange, byte rightRange, byte acrossRange, byte &rInter, byte &rRange) {
  // I'm going to avoid using sine and cosine, as those are heavy on a non-FPU machine
  float A, y, x, inter, range;

  float leftD = leftRange;
  float rightD = rightRange;
  float acrossD = acrossRange;

  const float HL = HEIGHT_LEN;
  const float BL = BASE_LEN;
  const float HBL = BL/2.0;

  // start by calculating x,y relative to Node_Light 20
  // get the area of the triangle by Heron's
  A = areaFromDistances(leftD, rightD, BL);
  Serial << F(" A=") << A;

  // determine the height of the triangle, given area and base
  y = yFromArea(A, BL);
  Serial << F(" y=") << y;
  
  // determine the base of the triangle, given height and hypotenuse
  x = xFromHeight(y, leftD);
  Serial << F(" x=") << x;
  
  // we then have the x,y coordinates relative to left vertex

  // compute the range of the object from the line, following the projected path from the sensor (cosine equivalence with right triangles)
  range = acrossD*(y/(HL-y));
  Serial << F(" r=") << range;
  
  // compute the projected intersection with the line (sine equivalence with right triangles)
  inter = x + range*(x-HBL)/acrossD;
  Serial << F(" i=") << inter;
  
  // update the message
  rInter = round(inter);
  rRange = round(range);

  Serial << endl;

  if( range>BASE_LEN ) { Serial << F("ERROR!  Range>BL") << endl; while(1); }

}

Location L;

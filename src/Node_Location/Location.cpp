#include "Location.h"

void Location::begin() {
  Serial << F("Location. startup with rangePin=") << rangePin << F(" and pwPin=") << pwPin << endl;

  pinMode(PW_PIN, INPUT);
  digitalWrite(RANGE_PIN, LOW);
  pinMode(RANGE_PIN, OUTPUT);

  this->calibrated = false;
}

byte Location::readDistance() {
  // do we need the initial calibration?
  if( !this->calibrated ) {
    this->calibrated = true;
    digitalWrite(RANGE_PIN, HIGH); // drive high, should take about 250 ms
    delay(350);
    digitalWrite(RANGE_PIN, LOW);
  }

  digitalWrite(RANGE_PIN, HIGH); // range
  float inches = pulseIn(PW_PIN, HIGH) / 147.0;
  digitalWrite(RANGE_PIN, LOW); // stop ranging

  return( inches>255 ? 255 : inches );
}

void Location::calculatePosition(Message msg) {
  // I'm going to avoid using sine and cosine, as those are heavy on a non-FPU machine
  float A, y, x, inter, range;
  const float HALF_BASE = ((float)BASE_LEN)/2.0;

  // start by calculating x,y relative to Node_Light 20
  // get the area of the triangle by Heron's
  A = areaFromDistances(msg.d[1], msg.d[2], BASE_LEN);

  // determine the height of the triangle, given area and base
  y = yFromArea(A, BASE_LEN);
  // determine the base of the triangle, given height and hypotenuse
  x = xFromHeight(y, msg.d[2]);

  // we then have the x,y coordinates relative to left vertex

  // compute the range of the object from the line, following the projected path from the sensor (cosine equivalence with right triangles)
  range = (float)msg.d[0]*((float)HEIGHT_LEN/((float)HEIGHT_LEN-y)-1.0);

  // compute the projected intersection with the line (sine equivalence with right triangles)
  inter = x + range*(x-HALF_BASE)/(float)msg.d[0];

  // update the message
  msg.range[0] = range>255 ? 255 : range;
  msg.inter[0] = inter>255 ? 255 : inter;

  // now x,y for Node_Light 21
  A = areaFromDistances(msg.d[2], msg.d[0], BASE_LEN);
  y = yFromArea(A, BASE_LEN);
  x = xFromHeight(y, msg.d[0]);
  range = (float)msg.d[1]*((float)HEIGHT_LEN/((float)HEIGHT_LEN-y)-1.0);
  inter = x + range*(x-HALF_BASE)/(float)msg.d[1];
  msg.range[1] = range>255 ? 255 : range;
  msg.inter[1] = inter>255 ? 255 : inter;
 
  // now x,y for Node_Light 22
  A = areaFromDistances(msg.d[0], msg.d[1], BASE_LEN);
  y = yFromArea(A, BASE_LEN);
  x = xFromHeight(y, msg.d[2]);
  range = (float)msg.d[1]*((float)HEIGHT_LEN/((float)HEIGHT_LEN-y)-1.0);
  inter = x + range*(x-HALF_BASE)/(float)msg.d[2];
  msg.range[2] = range>255 ? 255 : range;
  msg.inter[2] = inter>255 ? 255 : inter;
}

float Location::areaFromDistances(float lA, float lB, float lC) {
  float s = (lA + lB + lC) / 2.0;

  return ( pow( s * (s - lA) * (s - lB) * (s - lC), 0.5) );
}

float Location::yFromArea(float area, float base) {
  return ( 2.0 * area / base ); // base=this->length
}

float Location::xFromHeight(float y, float l) {
  return ( pow( pow(l, 2.0) - pow(y, 2.0), 0.5 ) ); // l is distance from vertex
}

Location L;

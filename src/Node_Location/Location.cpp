#include "Location.h"

void Location::begin(byte myNodeID) {
  Serial << F("Location. startup.") << endl;

  digitalWrite(PIN_GND, LOW); pinMode(PIN_GND, OUTPUT);
  pinMode(PIN_PW, INPUT);
  digitalWrite(PIN_RX, LOW); pinMode(PIN_RX, OUTPUT);
  digitalWrite(PIN_VCC, HIGH); pinMode(PIN_VCC, OUTPUT);

  // index accessor
  this->myIndex = N.whoAmI() - 10;
  Serial << F("Location.  storing distance information in messsage index=") << this->myIndex << endl;

  Serial << F("Location. delay for calibration") << endl;
  delay( ((unsigned long)this->myIndex)*1000UL );
  this->calibrateDistance();
}

void Location::readDistance(Message &msg) {
  // do we need the initial calibration?
  if ( !this->calibrated ) {
    this->calibrateDistance();
  }

  Serial << F("Location. ranging");
  unsigned long pulseTime = 0;
  while ( pulseTime == 0UL ) {
    Serial << F("r ");
    digitalWrite(PIN_RX, HIGH); // range
    pulseTime = pulseIn(PIN_PW, HIGH);
    digitalWrite(PIN_RX, LOW); // stop ranging
  }
  Serial << endl;

  // convert to distance in decainches; 1 inch = 100 decainches
  unsigned long decaInches; // promoted to 32-bit to handle *100 step later on

  // sanity checks
  if ( pulseTime == 0UL ) {
    Serial << F("Location. no reading from sensor.") << endl;
    decaInches = D_ERROR;
  }
  else if ( pulseTime > 37500UL ) {
    Serial << F("Location. out-of-range reading from sensor.") << endl;
    decaInches = D_ERROR;
  } else {
    // convert the number of us to decainches.  e.g. 1200 decainches is 12 inches.
    decaInches = ( pulseTime * 100 ) / 147; // max 25510.
  }

  Serial << F("Location. pulseTime (uS)=") << pulseTime << F(" range (decainches)=") << decaInches << F(" range (ft)=") << (float)decaInches/1200.0 << endl;
  msg.d[this->myIndex] = decaInches;
}

void Location::calibrateDistance() {
  Serial << F("Location.  calibrating range finder...") << endl;

  // depower the MaxSonar
  digitalWrite(PIN_VCC, LOW);
  // set the range pin to high
  digitalWrite(PIN_RX, HIGH);

  // wait for depower
  delay(100);

  // power the MaxSonar
  digitalWrite(PIN_VCC, HIGH);

  // wait for calibration; should take about 250 ms
  delay(200);

  // set the range pin to low
  digitalWrite(PIN_RX, LOW);

  // wait for another cycle, since we'll likely take a reading immediately hereafter
  delay(50);

  Serial << F("Location.  calibrated range finder.") << endl;

  this->calibrated = true;
}

void Location::calculatePosition(Message &msg) {

  // if any of the distance information is outside of the triangle, no need to calculate
  if ( msg.d[0] > BASE_LEN || msg.d[1] > BASE_LEN || msg.d[2] > BASE_LEN ) {
    Serial << F("Location. out-of-range.") << endl;
    msg.inter[0] = msg.inter[1] = msg.inter[2] = P_ERROR;
    msg.range[0] = msg.range[1] = msg.range[2] = P_ERROR;
    return;
  }

  // use Vivani's Theorem to adjust distances to centerpoint(ish)
  // see: https://en.wikipedia.org/wiki/Viviani%27s_theorem

  float totalD = (float)msg.d[0] + (float)msg.d[1] + (float)msg.d[2];
  const float heightD = sqrt(3.0)/2.0*(float)SENSOR_DIST;
  float bumpD = heightD/totalD;
  
  Serial << F("Location. calculatePosition A20: ");
  //  heavyLift(msg.d[2], msg.d[1], msg.d[0], msg.inter[0], msg.range[0]);
  simpleLift(msg.d[2], msg.d[1], msg.inter[0], msg.range[0]);

  Serial << endl << F("Location. calculatePosition A21: ");
  //  heavyLift(msg.d[0], msg.d[2], msg.d[1], msg.inter[1], msg.range[1]);
  simpleLift(msg.d[0], msg.d[2], msg.inter[1], msg.range[1]);

  Serial << endl << F("Location. calculatePosition A22: ");
  //  heavyLift(msg.d[1], msg.d[0], msg.d[2], msg.inter[2], msg.range[2]);
  simpleLift(msg.d[1], msg.d[0], msg.inter[2], msg.range[2]);

  Serial << endl;
}

/*
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

void Location::heavyLift(word leftRange, word rightRange, word acrossRange, word &rInter, word &rRange) {
  // I'm going to avoid using sine and cosine, as those are heavy on a non-FPU machine
  float A, y, x, inter, range;

  float leftD = leftRange / 100.0;
  float rightD = rightRange / 100.0;
  float acrossD = acrossRange / 100.0;

  const float HL = HEIGHT_LEN;
  const float BL = BASE_LEN;
  const float HBL = BL / 2.0;

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
  range = acrossD * (y / (HL - y));
  Serial << F(" r=") << range;

  // compute the projected intersection with the line (sine equivalence with right triangles)
  inter = x + range * (x - HBL) / acrossD;
  Serial << F(" i=") << inter;

  // update the message
  rInter = round(inter * 100.0);
  rRange = round(range * 100.0);

  Serial << endl;

  if ( rInter == 0 || rInter > BASE_LEN || rRange == 0 || rRange > BASE_LEN) {
    rInter = P_ERROR;
    rRange = P_ERROR;
  }

}
*/

// From: http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
/**
 * \brief    Fast Square root algorithm, with rounding
 *
 * This does arithmetic rounding of the result. That is, if the real answer
 * would have a fractional part of 0.5 or greater, the result is rounded up to
 * the next integer.
 *      - SquareRootRounded(2) --> 1
 *      - SquareRootRounded(3) --> 2
 *      - SquareRootRounded(4) --> 2
 *      - SquareRootRounded(6) --> 2
 *      - SquareRootRounded(7) --> 3
 *      - SquareRootRounded(8) --> 3
 *      - SquareRootRounded(9) --> 3
 *
 * \param[in] a_nInput - unsigned integer for which to find the square root
 *
 * \return Integer square root of the input value.
 */
uint32_t SquareRootRounded(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }

    /* Do arithmetic rounding to nearest integer */
    if (op > res)
    {
        res++;
    }

    return res;
}



void Location::simpleLift(word leftRange, word rightRange, word &rInter, word &rRange) {
  // I'm going to avoid using sine and cosine, as those are heavy on a non-FPU machine
  // see https://en.m.wikipedia.org/wiki/Heron%27s_formula "Algebraic proof using the Pythagorean theorem"

  // first, some error checking.

  // is leftRange or rightRange > BASE_LEN?  If, so, we're outside of the triagle.
  if ( leftRange > (unsigned long)BASE_LEN || rightRange > (unsigned long)BASE_LEN ) {
    rInter = P_ERROR;
    rRange = P_ERROR;
    return;
  }

  const unsigned long cSq = (unsigned long)SENSOR_DIST * (unsigned long)SENSOR_DIST;
  const unsigned long cTwo = (unsigned long)SENSOR_DIST * (unsigned long)2;
  unsigned long lSq = (unsigned long)leftRange * (unsigned long)leftRange;
  unsigned long rSq = (unsigned long)rightRange * (unsigned long)rightRange;

//  unsigned long d = (-rSq + lSq + cSq) / cTwo;
  unsigned long d = ((lSq + cSq)-rSq) / cTwo;
  Serial << F(" d=") << d;
  
  if ( d > (unsigned long)SENSOR_DIST ) {
    // error
    rInter = P_ERROR;
    rRange = P_ERROR;
    return;
  }

  Serial << F(" l^2=") << lSq << F(" d^2=") << d*d;
  unsigned long d2 = d * d;
  unsigned long h;
  if( lSq > d2 ) {
    unsigned long hSq = lSq - d2;
    Serial << F(" h^2=") << hSq;
    h = SquareRootRounded(hSq);
    Serial << F(" h=") << h;
  } else {
    h = 0; // right at the tube
  }
  
  if ( h > (unsigned long)BASE_LEN ) {
     Serial << F("Sensor error") << endl;
    while(1);
   // error
    rInter = P_ERROR;
    rRange = P_ERROR;
    return;
  }

  rInter = d;
  rRange = h;
}

Location L;

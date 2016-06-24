#include "Location.h"

void Location::begin(byte myNodeID) {
  Serial << F("Location. startup.") << endl;

  digitalWrite(PIN_GND, LOW); pinMode(PIN_GND, OUTPUT);
  pinMode(PIN_PW, INPUT);

  digitalWrite(PIN_RX, LOW); pinMode(PIN_RX, OUTPUT);
//  digitalWrite(PIN_RX, LOW);
//  pinModeFast(PIN_RX,OUTPUT);

  digitalWrite(PIN_VCC, HIGH); pinMode(PIN_VCC, OUTPUT);

  // index accessor
  this->myIndex = N.whoAmI() - 10;
  Serial << F("Location.  storing distance information in messsage index=") << this->myIndex << endl;

  Serial << F("Location. delay for calibration") << endl;
  delay( ((unsigned long)this->myIndex)*1000UL );
  this->calibrateDistance();

  maxDist[0] = maxDist[1] = maxDist[2] = HEIGHT_LEN;
}

void Location::readDistance(Message &msg) {
  // do we need the initial calibration?
  if ( !this->calibrated ) {
    this->calibrateDistance();
  }

//  Serial << F("Location. ranging");
/*
  const byte N_samples = 3;
  unsigned long pulseTime[N_samples];
//  digitalWrite(PIN_RX, HIGH); // range
//  digitalWriteFast(PIN_RX,HIGH);

  for( byte i=0; i<N_samples; i++ ) {
    pulseTime[i] = pulseIn(PIN_PW, HIGH);
  }
//  digitalWrite(PIN_RX, LOW); // stop ranging
//  digitalWriteFast(PIN_RX, LOW);

  // constrain spurious readings to be within triangle
  const unsigned long maxTime = 147.0*(float)HEIGHT_LEN/100.0;
  unsigned long sumTime = 0;
  for( byte i=0; i<N_samples; i++ ) {
    Serial << F("Location.  pulseTime=") << pulseTime[i] << endl;
//    sumTime += pulseTime[i] > maxTime ? maxTime : pulseTime[i];
    sumTime += pulseTime[i];
  }
  // average
  unsigned long avgTime = sumTime / N_samples;

  // convert to distance in decainches; 1 inch = 100 decainches
  unsigned long decaInches = ( avgTime * 100 ) / 147; // max HEIGHT_LEN
 
//  Serial << F("Location. pulseTime (uS)=") << pulseTime << F(" range (decainches)=") << decaInches << F(" range (ft)=") << (float)decaInches/1200.0 << endl;
  msg.d[this->myIndex] = decaInches;
*/

  // record the PW length
  digitalWrite(PIN_RX, HIGH); // range
  unsigned long pulseTime = pulseIn(PIN_PW, HIGH);
//  pulseTime = pulseIn(PIN_PW, HIGH);
  digitalWrite(PIN_RX, LOW); // stop ranging

  // convert to distance in decainches; 1 inch = 100 decainches
  unsigned long decaInches = ( pulseTime * 100 ) / 147; // max HEIGHT_LEN

  // record 
  msg.d[this->myIndex] = decaInches;
  /*
  while ( pulseTime == 0UL ) {
//    Serial << F("r ");
    digitalWrite(PIN_RX, HIGH); // range
    pulseTime = pulseIn(PIN_PW, HIGH);
    digitalWrite(PIN_RX, LOW); // stop ranging
  }
//  Serial << endl;
*/
/*
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
*/
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

void cornerCase(Message &msg, byte aI, byte rI, byte lI) {
    // across
    msg.inter[aI] = SENSOR_DIST/2;
    msg.range[aI] = HEIGHT_LEN - IN_CORNER;
    // to the right
    msg.inter[rI] = IN_CORNER/2;
    msg.range[rI] = IN_CORNER/2;
    // to the left
    msg.inter[lI] = SENSOR_DIST - IN_CORNER/2;
    msg.range[lI] = IN_CORNER/2;
}

void Location::calculatePosition(Message &msg) {

  // corner cases... literally.
  if( msg.d[0]<=IN_CORNER ) {
    cornerCase(msg, 0, 1, 2);
    return;
  }
  if( msg.d[1]<=IN_CORNER ) {
    cornerCase(msg, 1, 2, 0);
    return;
  }
  if( msg.d[2]<=IN_CORNER ) {
    cornerCase(msg, 2, 0, 1);
    return;
  }

  // use left and right sensors to determine intercept at base
  msg.inter[0] = SENSOR_DIST - intercept(msg.d[2], msg.d[1]);
  msg.inter[1] = SENSOR_DIST - intercept(msg.d[0], msg.d[2]);
  msg.inter[2] = SENSOR_DIST - intercept(msg.d[1], msg.d[0]);

  // use left sensor and intercept to determine height
//  msg.range[0] = height(msg.d[2], msg.inter[0]);
//  msg.range[1] = height(msg.d[0], msg.inter[1]);
//  msg.range[2] = height(msg.d[1], msg.inter[2]);

  // use the across sensor to approximate height
  for( byte i=0; i<3; i++ ) {
    if( msg.d[i] < HEIGHT_LEN ) {
      msg.range[i] = HEIGHT_LEN - msg.d[i];
    } else {
      msg.range[i] = 0;
    }
  }
  /*
  // reset range calculations
  msg.range[0] = msg.range[1] = msg.range[2] = P_ERROR;

  // count the intercept errors
  byte interError = 0;
  for( byte i=0; i<3; i++) {
    if( msg.inter[i] == P_ERROR ) {
      interError++;
    }
  }

  if( interError == 3 ) {
    // uh, that's a problem.
    return;
  }

  if( interError == 2 ) {
    // must be in a corner
    // should be able to do something here, but bail out for now
    return;
  }

  if( interError == 1 ) {
    byte fixThis = 0;
    if( msg.inter[1] == P_ERROR ) fixThis = 1;
    if( msg.inter[2] == P_ERROR ) fixThis = 2;
    // use Vivani's theorem: https://en.wikipedia.org/wiki/Viviani%27s_theorem
    msg.inter[fixThis] = msg.d[fixThis]-
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

  */
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
unsigned long Location::SquareRootRounded(unsigned long a_nInput) {
    unsigned long op  = a_nInput;
    unsigned long res = 0;
    unsigned long one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

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

word Location::intercept(unsigned long lR, unsigned long rR) {
  
  // see https://en.m.wikipedia.org/wiki/Heron%27s_formula "Algebraic proof using the Pythagorean theorem"
  const unsigned long cSq = (unsigned long)SENSOR_DIST * (unsigned long)SENSOR_DIST;
  const unsigned long cTwo = (unsigned long)SENSOR_DIST * (unsigned long)2;

  unsigned long lSq = lR * lR;
  unsigned long rSq = rR * rR;

  unsigned long d = ((lSq + cSq)-rSq) / cTwo;

  d = d>(unsigned long)SENSOR_DIST ? (unsigned long)SENSOR_DIST : d;

  return( d ); 
}

word Location::height(unsigned long lR, unsigned long intercept) {
  
  unsigned long lSq = lR * lR;
  unsigned long iSq = intercept * intercept;
  unsigned long h;
  if( lSq > iSq ) {
    unsigned long hSq = lSq - iSq;
//    Serial << F(" h^2=") << hSq;
    h = SquareRootRounded(hSq);
//    Serial << F(" h=") << h;
  } else {
    h = lR; // in the corner
  }

  return( h );
}


word Location::heightAlt(unsigned long aR) {  
  return( HEIGHT_LEN - aR );
}


/*
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
*/

Location L;

#include "Location.h"

void Location::boot(byte sIndex, byte EN_PIN, byte RNG_PIN, byte PW_PIN) {

  Serial << "Location.  Booting sensor " << sIndex << endl;

  // for each sensor
  digitalWrite(RNG_PIN, LOW);
  digitalWrite(EN_PIN, LOW);
  // I'm going to set the PW pin LOW, too, in case that's keeping the board powered
  digitalWrite(PW_PIN, LOW);

  pinMode(RNG_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(PW_PIN, OUTPUT);

  // wait for depower
  delay(500);

  // calibrate
  digitalWrite(EN_PIN, HIGH);
  digitalWrite(RNG_PIN, HIGH);
  delay(300);

  // turn it off
  digitalWrite(RNG_PIN, LOW);
  delay(100);

  // now, flip the PW pin back to INPUT
  pinMode(PW_PIN, INPUT);
}

void Location::begin(Distances *D) {
  Serial << F("Location. startup.") << endl;

  for ( byte i = 0; i < N_RANGE; i++ ) {
    // boot sensor
    boot(i, PIN_EN[i], PIN_RNG[i], PIN_PW[i]);
    // set ranges to reasonable values
    currRange[i] = HL;
    avgRange[i] = HL;
  }


  this->d = D;

  Serial << F("Location. startup complete.") << endl;
}

word range(byte RNG_PIN, byte PW_PIN) {

  digitalWrite(RNG_PIN, HIGH);
  unsigned long val = pulseIn(PW_PIN, HIGH);
  digitalWrite(RNG_PIN, LOW);

  // constrained to HL
  return ( (word)constrain(val, 0, HL) );
}

void Location::update(byte smoothing) {
  for ( byte i = 0; i < N_RANGE; i++ ) {
    currRange[i] = range(PIN_RNG[i], PIN_PW[i]);

    avgRange[i] = (currRange[i] + avgRange[i] * smoothing) / (1 + smoothing);
  }
}

void Location::calculateLocation() {
  // the distance measures to the right and left of the LED strips and
  // the known distance between the sensors form a triangle.
  for (byte i = 0; i < N_NODES; i++) d->D[Index[i]] = avgRange[i];

  // altitude height.  the extent of the triangle's altitude.
  // these are the trilinear coordinates of the object.
  // https://en.wikipedia.org/wiki/Trilinear_coordinates
  for (byte i = 0; i < N_NODES; i++) d->Ah[i] = altitudeHeight(i);

  // use Vivani's theorem to adjust the sum of the heights to total height.
  correctAltitudeHeight();

  // altitude base.  the distance of the triangle's altitude along the side length.
  for (byte i = 0; i < N_NODES; i++) d->Ab[i] = altitudeBase(i);

  // from here, we want to know the location on the side length that is
  // collinear with the opposite sensor and the object. that is, a
  // cevian from the opposite vertex that passes through the object.
  // https://en.wikipedia.org/wiki/Cevian

  // collinear base. the distance of the collinear point along the side length.
  for (byte i = 0; i < N_NODES; i++) d->Cb[i] = collinearBase(i);

  // collinear height. the distance between the collinear point and the object.
  for (byte i = 0; i < N_NODES; i++) d->Ch[i] = collinearHeight(i);

  // fractional area. this is the barycentric coordinate relative to the opposite vertex.
  for (byte i = 0; i < N_NODES; i++) d->Area[i] = area(i);

}

// semiperimiter calculation
word Location::semiPerimeter(byte i) {
  unsigned long twoSP = SL + d->D[left(i)] + d->D[right(i)];
  return ( twoSP >> 1 ); // >>1 is /2
}

// given the altitude's distance along the side length and the distance,
// use Pythagorean's theorem to calculte the altitude
word Location::altitudeHeight(byte i) {
  // use right-i and left-i distance
  if ( d->D[left(i)] + d->D[right(i)] < SL ) return ( 0 );

  unsigned long s = semiPerimeter(i);
  unsigned long part1 = squareRoot(s * (s - d->D[left(i)]));
  unsigned long part2 = squareRoot((s - SL) * (s - d->D[right(i)]));
  //  Serial << F("i=") << i << F("\ts=") << s << F("\tpart1=") << part1 << F("\tpart2=") << part2 << endl;

  unsigned long tmp = 2UL * part1 * part2;
  return ( tmp / (unsigned long)SL );
}

// use Vivani's theorem to adjust the sum of the heights to total height.
// briefly, the the altitudes of the three triangles defined
// with the object as a vertex must sum to the total triangle height
void Location::correctAltitudeHeight() {

  // compute the difference in sum of heights from HL
  int deltaAh = HL;
  for (byte i = 0; i < N_NODES; i++) deltaAh -= d->Ah[i];
  //  Serial << F("deltaAh=") << deltaAh << endl;

  // are we done?
  if ( deltaAh == 0 ) return;

  //  Serial << F("Ah=\t") << Ah[0] << F("\t") << Ah[1] << F("\t") << Ah[2] << endl;

  // what do we do with it?
  if ( deltaAh < 3 && deltaAh > -3 ) {
    // small amount, so finish it
    if ( d->Ah[0] < d->Ah[1] && d->Ah[0] < d->Ah[2] ) d->Ah[0] += deltaAh;
    else if ( d->Ah[1] < d->Ah[0] && d->Ah[1] < d->Ah[2] ) d->Ah[1] += deltaAh;
    else d->Ah[2] += deltaAh;

    //    Serial << F("Ah=\t") << Ah[0] << F("\t") << Ah[1] << F("\t") << Ah[2] << endl;
    return;
  } else {
    // dole that difference out equally.
    int addTo = deltaAh / 3; // this division operation is slow
    for (byte i = 0; i < N_NODES; i++) d->Ah[i] += addTo;

    // and do this again to grab remainder
    correctAltitudeHeight();
  }
}


// given two distances and a known side length, we can calculate the
// the altitude's distance along the side length of the altitude.
// from https://en.wikipedia.org/wiki/Heron%27s_formula#Algebraic_proof_using_the_Pythagorean_theorem
word Location::altitudeBase(byte i) {
  if ( d->D[left(i)] < d->Ah[i] ) return (0);

  // use left-i distance and ith height
  return (
           squareRoot( squared(d->D[left(i)]) - squared(d->Ah[i]) )
         );
}


// given the left and right altitudes to the object, we leverage these
// trilinear coordinates to express the colinear point's location
// on the side length. trilinear coordinates, in particular, enable an
// algebraic solution involving determinants. otherwise, we'd require
// calculations of angles.
// https://en.wikipedia.org/wiki/Trilinear_coordinates#Collinearities_and_concurrencies
word Location::collinearBase(byte i) {
  // use left-i altitude height and right-i altitude height
  return (
           ((unsigned long)(d->Ah[right(i)]) * SL) / ((unsigned long)d->Ah[right(i)] + (unsigned long)d->Ah[left(i)])
         );
}

// given the extent of the altitude and the difference beween the collinear
// base intercept and the altitude's base intercept, we can use
// Pythagorean's theorem to compute the length of the collinear line.
word Location::collinearHeight(byte i) {
  // use i altitude height and base; i collinear base
  return ( d->Cb[i] >= d->Ab[i] ? // unsigned, so careful of order
           squareRoot( squared(d->Ah[i]) + squared(d->Cb[i] - d->Ab[i]) ) :
           squareRoot( squared(d->Ah[i]) + squared(d->Ab[i] - d->Cb[i]) )
         );
}

// finally, we can convert the trilinear coordinates to barycentric coordinates,
// which immediately gives us the FRACTION of the total triangle area
// occupied by the triangle above this LED segment.
// interpret the return of this function to be 65534ths
// https://en.wikipedia.org/wiki/Barycentric_coordinate_system
word Location::area(byte i) {
  // use i altitude height
  // multiply the ratio of Ah[i]/sum(Ah[]) by the maximum word value to
  // generate a 8-bit fractional result.
  return (
           ((unsigned long)((word)(-1)) * (unsigned long)d->Ah[i]) / (unsigned long)SL
         );
}


unsigned long Location::squared(word x) {
  // taking care to promote operand
  return (
           (unsigned long)x * (unsigned long)x
         );
}

// From: http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
word Location::squareRoot(unsigned long x) {
  unsigned long op  = x;
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

  return (word)res;
}

// to index left is index+1
byte Location::left(byte i) {
  // get the tens digit
  byte tens = i - (i % 10);
  // get the ones digit
  byte ones = i - tens;

  return ( tens + ones < 2 ? ones + 1 : 0 );
}

// to index right is index-1
byte Location::right(byte i) {
  // get the tens digit
  byte tens = i - (i % 10);
  // get the ones digit
  byte ones = i - tens;

  return ( tens + ones > 0 ? ones - 1 : 2 );
}



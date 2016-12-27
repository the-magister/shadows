#include "Location.h"

void Location::begin(Distances *D) {
  Serial << F("Location. startup.");

  // has the effect of stopping any ongoing round-robin
  digitalWrite(PIN_START_RANGE, LOW); 
  pinMode(PIN_START_RANGE, OUTPUT);

  //  Timing Description
  // 250mS after power-up, the LV-MaxSonar-EZ is ready to accept the RX command. 
  delay(500); // delay after power up

  // AN Output Constantly Looping:
  // "To start the continuous loop, bring the RX pin high for a time greater than 20us but 
  // less than 48ms and return to ground."
  
  digitalWrite(PIN_START_RANGE, HIGH);
  delay(10);
  digitalWrite(PIN_START_RANGE, LOW);
  
  pinMode(PIN_START_RANGE, INPUT); // flip to high-impediance pin state so as to not clobber the return round-robin inc.

  this->d = D;

//  analogReference(DEFAULT);
  analogReference(INTERNAL); // built-in reference, which is 1.1V on ATmega328
  // readings are capping out at 220 with a 5V reference, or about 1.078V.
  // so, the 1.1V reference is nearly perfect.  

  // reset min and max readings
  for( byte i=0; i<N_RANGE; i++ ) {
    maxReading[i] = 1000;
    minReading[i] = 20;
  }
  
  Serial << F("Location. startup complete.") << endl;
}


void Location::update() {
  // store readings
  static unsigned long reading[N_RANGE] = { 1023, 1023, 1023 }; // initialize with a reasonable set of values
  // over this interval of time
  Metro updateTime(5UL);
  // It takes about 100 microseconds (0.0001 s) to read an analog input, so should get about 50 readings.
  
  while( !updateTime.check() ) {
    // do a running average, updating the value by 1/256ths with each reading
    for( byte n=0; n<N_RANGE; n++ ) reading[n] = (255UL*(unsigned long)reading[n] + analogRead(rangePin[n])) >> 8; // >>8 is /256
  }
    
  // AN Output:
  // Outputs analog voltage with a scaling factor of (Vcc/512) per inch. 
  // A supply of 5V yields ~9.8mV/in. and 3.3V yields ~6.4mV/in. 
  // The output is buffered and corresponds to the most recent range data.
  
  // Arduino analogRead:
  // This means that it will map input voltages between 0 and 5 volts into integer 
  // values between 0 and 1023. This yields a resolution between readings of: 
  // 5 volts / 1024 units or, .0049 volts (4.9 mV) per unit.
  
  // To Mike's reading, this means a unit of "1" in the analog read is 0.5 in.
  // distance [in] = reading [unit] * (5/1024 [volts/unit]) / (5/512 [volts/in])
  // distance [in] = reading [unit] * (1/2 [in/unit])

  for( byte i=0; i<N_RANGE; i++ ) {
    if( reading[i] > maxReading[i] ) maxReading[i] = reading[i];
    if( reading[i] < minReading[i] ) minReading[i] = reading[i];
      
    d->D[i] = map( reading[i], minReading[i], maxReading[i], 0, HL);
  }

  // update the locations
  calculateLocation();
}



void Location::calculateLocation() {
  // the distance measures to the right and left of the LED strips and
  // the known distance between the sensors form a triangle.

  // cap the distance readings
  // DANNE, I think we just need to cap the readings to not exceed HL to prevent Bad Math Happening.
  for(byte i=0; i<N_RANGE; i++) {
    if( d->D[i] > HL ) d->D[i] = HL;
  }

  // altitude height.  the extent of the triangle's altitude.
  // these are the trilinear coordinates of the object.
  // https://en.wikipedia.org/wiki/Trilinear_coordinates
  for(byte i=0; i<N_NODES; i++) d->Ah[i] = altitudeHeight(i);

  // use Vivani's theorem to adjust the sum of the heights to total height.
  correctAltitudeHeight();
  
  // altitude base.  the distance of the triangle's altitude along the side length.
  for(byte i=0; i<N_NODES; i++) d->Ab[i] = altitudeBase(i);
  
  // from here, we want to know the location on the side length that is
  // collinear with the opposite sensor and the object. that is, a 
  // cevian from the opposite vertex that passes through the object.
  // https://en.wikipedia.org/wiki/Cevian
  
  // collinear base. the distance of the collinear point along the side length.
  for(byte i=0; i<N_NODES; i++) d->Cb[i] = collinearBase(i);
  
  // collinear height. the distance between the collinear point and the object.
  for(byte i=0; i<N_NODES; i++) d->Ch[i] = collinearHeight(i);
  
  // fractional area. this is the barycentric coordinate relative to the opposite vertex.
  for(byte i=0; i<N_NODES; i++) d->Area[i] = area(i);
  
}

// semiperimiter calculation
word Location::semiPerimeter(byte i) {
  unsigned long twoSP = SL + d->D[left(i)] + d->D[right(i)];
  return( twoSP >> 1 ); // >>1 is /2
}

// given the altitude's distance along the side length and the distance,
// use Pythagorean's theorem to calculte the altitude 
word Location::altitudeHeight(byte i) {
  // use right-i and left-i distance
  if( d->D[left(i)] + d->D[right(i)] < SL ) return( 0 );

  unsigned long s = semiPerimeter(i);
  unsigned long part1 = squareRoot(s*(s - d->D[left(i)]));
  unsigned long part2 = squareRoot((s-SL)*(s - d->D[right(i)]));
//  Serial << F("i=") << i << F("\ts=") << s << F("\tpart1=") << part1 << F("\tpart2=") << part2 << endl;


  unsigned long tmp = 2UL * part1 * part2;
  if( SL != 65535 ) {
    Serial << F("BAD MATH!!!  (fix me)") << endl;
    while(1);
  }
  return( tmp >> 16 ); // >>16 is /65536 which is SL
}

// use Vivani's theorem to adjust the sum of the heights to total height.
// briefly, the the altitudes of the three triangles defined
// with the object as a vertex must sum to the total triangle height
void Location::correctAltitudeHeight() {

  // compute the difference in sum of heights from HL
  int deltaAh = HL;
  for(byte i=0; i<N_NODES; i++) deltaAh -= d->Ah[i];
//  Serial << F("deltaAh=") << deltaAh << endl;
  
  // are we done?
  if( deltaAh == 0 ) return;
  
//  Serial << F("Ah=\t") << Ah[0] << F("\t") << Ah[1] << F("\t") << Ah[2] << endl;
  
  // what do we do with it?
  if( deltaAh<3 && deltaAh>-3 ) {
    // small amount, so finish it
    if( d->Ah[0] < d->Ah[1] && d->Ah[0] < d->Ah[2] ) d->Ah[0]+=deltaAh;
    else if( d->Ah[1] < d->Ah[0] && d->Ah[1] < d->Ah[2] ) d->Ah[1]+=deltaAh;
    else d->Ah[2]+=deltaAh;

//    Serial << F("Ah=\t") << Ah[0] << F("\t") << Ah[1] << F("\t") << Ah[2] << endl;
    return;
  } else {
    // dole that difference out equally.
    int addTo = deltaAh/3; // this division operation is slow
    for(byte i=0; i<N_NODES; i++) d->Ah[i] += addTo;
    
    // and do this again to grab remainder
    correctAltitudeHeight();
  }
}


// given two distances and a known side length, we can calculate the 
// the altitude's distance along the side length of the altitude.
// from https://en.wikipedia.org/wiki/Heron%27s_formula#Algebraic_proof_using_the_Pythagorean_theorem
word Location::altitudeBase(byte i) {
  if( d->D[left(i)] < d->Ah[i] ) return(0);
  
  // use left-i distance and ith height
  return(
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
  return(
    ((unsigned long)(d->Ah[right(i)])*SL)/((unsigned long)d->Ah[right(i)]+(unsigned long)d->Ah[left(i)])
  );
}

// given the extent of the altitude and the difference beween the collinear 
// base intercept and the altitude's base intercept, we can use 
// Pythagorean's theorem to compute the length of the collinear line. 
word Location::collinearHeight(byte i) {
  // use i altitude height and base; i collinear base
  return( d->Cb[i] >= d->Ab[i] ? // unsigned, so careful of order
    squareRoot( squared(d->Ah[i]) + squared(d->Cb[i]-d->Ab[i]) ) :
    squareRoot( squared(d->Ah[i]) + squared(d->Ab[i]-d->Cb[i]) )
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
  return(
    ((unsigned long)((word)(-1))*(unsigned long)d->Ah[i]) / (unsigned long)SL
  );
}


unsigned long Location::squared(word x) {
  // taking care to promote operand 
  return(
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
  
  return( tens + ones<2 ? ones+1 : 0 );
}

// to index right is index-1
byte Location::right(byte i) {
  // get the tens digit
  byte tens = i - (i % 10); 
  // get the ones digit
  byte ones = i - tens;
  
  return( tens + ones>0 ? ones-1 : 2 );
}



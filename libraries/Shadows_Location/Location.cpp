#include "Location.h"

void Location::begin(byte *distance) {
	Serial << F("Location. startup.");
	this->distance = distance;
	Serial << F("\td0=") << this->distance[0];
	Serial << F("\td1=") << this->distance[1];
	Serial << F("\td2=") << this->distance[2];
	Serial << endl;
	Serial << F("Location. startup complete.") << endl;
}

void Location::calculateLocation() {
	// the distance measures to the right and left of the LED strips and
	// the known distance between the sensors form a triangle.

	// altitude height.  the extent of the triangle's altitude.
	// these are the trilinear coordinates of the object.
	// https://en.wikipedia.org/wiki/Trilinear_coordinates
	for(byte i=0; i<N_NODES; i++) Ah[i] = altitudeHeight(i);

	// use Vivani's theorem to adjust the sum of the heights to total height.
	correctAltitudeHeight();
	
	// altitude base.  the distance of the triangle's altitude along the side length.
	for(byte i=0; i<N_NODES; i++) Ab[i] = altitudeBase(i);
	
	// from here, we want to know the location on the side length that is
	// collinear with the opposite sensor and the object. that is, a 
	// cevian from the opposite vertex that passes through the object.
	// https://en.wikipedia.org/wiki/Cevian
	
	// collinear base. the distance of the collinear point along the side length.
	for(byte i=0; i<N_NODES; i++) Cb[i] = collinearBase(i);
	
	// collinear height. the distance between the collinear point and the object.
	for(byte i=0; i<N_NODES; i++) Ch[i] = collinearHeight(i);
	
	// fractional area. this is the barycentric coordinate relative to the opposite vertex.
	for(byte i=0; i<N_NODES; i++) Area[i] = area(i);
	
}

// semiperimiter calculation
word Location::semiPerimeter(byte i) {
	return(
		( SL + distance[left(i)] + distance[right(i)] ) >> 1 // >>1 is /2
	);
}

// given the altitude's distance along the side length and the distance,
// use Pythagorean's theorem to calculte the altitude 
word Location::altitudeHeight(byte i) {
	// use right-i and left-i distance
	if( distance[left(i)] + distance[right(i)] < SL ) return( 0 );

	unsigned long s = semiPerimeter(i);
	unsigned long part1 = squareRoot(s*(s-distance[left(i)]));
	unsigned long part2 = squareRoot((s-SL)*(s-distance[right(i)]));
//	Serial << F("i=") << i << F("\ts=") << s << F("\tpart1=") << part1 << F("\tpart2=") << part2 << endl;
	
	return( 
		(2UL * part1 * part2) / SL // this big divide operator is slow.  bummer.
	);
}

// use Vivani's theorem to adjust the sum of the heights to total height.
// briefly, the the altitudes of the three triangles defined
// with the object as a vertex must sum to the total triangle height
void Location::correctAltitudeHeight() {

	// compute the difference in sum of heights from SL
	int deltaAh = HL;
	for(byte i=0; i<N_NODES; i++) deltaAh -= Ah[i];
//	Serial << F("deltaAh=") << deltaAh << endl;
	
	// are we done?
	if( deltaAh == 0 ) return;
	
//	Serial << F("Ah=\t") << Ah[0] << F("\t") << Ah[1] << F("\t") << Ah[2] << endl;
	
	// what do we do with it?
	if( deltaAh<3 && deltaAh>-3 ) {
		// small amount, so finish it
		if( Ah[0] < Ah[1] && Ah[0] < Ah[2] ) Ah[0]+=deltaAh;
		else if( Ah[1] < Ah[0] && Ah[1] < Ah[2] ) Ah[1]+=deltaAh;
		else Ah[2]+=deltaAh;

//		Serial << F("Ah=\t") << Ah[0] << F("\t") << Ah[1] << F("\t") << Ah[2] << endl;
		return;
	} else {
		// dole that difference out equally.
		int addTo = deltaAh/3; // this division operation is slow
		for(byte i=0; i<N_NODES; i++) Ah[i] += addTo;
		
		// and do this again to grab remainder
		correctAltitudeHeight();
	}
}


// given two distances and a known side length, we can calculate the 
// the altitude's distance along the side length of the altitude.
// from https://en.wikipedia.org/wiki/Heron%27s_formula#Algebraic_proof_using_the_Pythagorean_theorem
word Location::altitudeBase(byte i) {
	if( distance[left(i)] < Ah[i] ) return(0);
	
	// use left-i distance and ith height
	return(
		squareRoot( squared(distance[left(i)]) - squared(Ah[i]) )
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
		((unsigned long)(Ah[right(i)])*SL)/((unsigned long)Ah[right(i)]+(unsigned long)Ah[left(i)])
	);
}

// given the extent of the altitude and the difference beween the collinear 
// base intercept and the altitude's base intercept, we can use 
// Pythagorean's theorem to compute the length of the collinear line. 
word Location::collinearHeight(byte i) {
	// use i altitude height and base; i collinear base
	return( Cb[i] >= Ab[i] ? // unsigned, so careful of order
		squareRoot( squared(Ah[i]) + squared(Cb[i]-Ab[i]) ) :
		squareRoot( squared(Ah[i]) + squared(Ab[i]-Cb[i]) )
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
	// generate a 16-bit fractional result.
	return(
		((unsigned long)((word)(-1))*(unsigned long)Ah[i]) / (unsigned long)SL
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

Location L;

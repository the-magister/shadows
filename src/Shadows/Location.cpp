#include "Location.h"

void Location::begin(byte left, byte across, byte right, long length) {
	Serial << F("Location. startup with leftNode=") << leftNode;
	Serial << F(" acrossNode=") << acrossNode;
	Serial << F(" leftNode=") << acrossNode;
	Serial << endl;

	Serial << F("Location. startup with length=") << length << endl;
	
	// https://en.wikipedia.org/wiki/Equilateral_triangle
	this->length = length;
	
	this->leftNode = left;
	this->acrossNode = across;
	this->rightNode = right;
	
	this->distanceCalculated = false;
}

void Location::setDistance(byte node, long distance) {
	if( node == leftNode ) {
		this->leftD = distance;
		this->distanceCalculated = false;
	}
	if( node == acrossNode ) {
		this->acrossD = distance;
		this->distanceCalculated = false;
	}
	if( node == rightNode ) {
		this->rightD = distance;
		this->distanceCalculated = false;
	}
}

boolean Location::isObject(long maxDistance) {
	if( this->leftD<=maxDistance && this->acrossD<=maxDistance && this->rightD<=maxDistance ) {
		return( true );
	} else {
		return( false );
	}
}

float Location::areaFromDistances(float lA, float lB, float lC) {
	float s = (lA+lB+lC)/2.0;
	
	return( pow( s*(s-lA)*(s-lB)*(s-lC), 0.5) );
}

float Location::yFromArea(float area, float base) {
	return( 2.0*area/base ); // base=this->length
}

float Location::xFromHeight(float y, float l) {
	return( pow( pow(l,2.0)-pow(y,2.0), 0.5 ) ); // l is distance from vertex
}

void Location::triangulate() {
	// I'm going to avoid using sine and cosine, as those are heavy on a non-FPU machine
	
	// get the area of the triangle by Heron's 
	float A = areaFromDistances(this->leftD, this->rightD, this->length);
	
	// determine the height of the triangle, given area and base
	float y = yFromArea(A, this->length);
	// by definition, our midpoint has zero height
	
	// determine the base of the triangle, given height and hypotenuse
	float x;
	// use the longer hypotenuese
	if( this->leftD >= this->rightD ) {
		x = xFromHeight(y, this->leftD);
		// and make this relative to our midpoint
		x = x - ((float)this->length)/2.0;
	} else {
		x = xFromHeight(y, this->rightD);
		// and make this relative to our midpoint
		x = ((float)this->length)/2.0 - x;
	}
	
	// store this expensive calculations
	this->x = x;
	this->y = y;
	distanceCalculated = true;
}

long Location::xPosition() {
	if( ! this->distanceCalculated ) triangulate();
	
	return( this->x );
}

long Location::yPosition() {
	if( ! this->distanceCalculated ) triangulate();
	
	return( this->y );
}

Location L;

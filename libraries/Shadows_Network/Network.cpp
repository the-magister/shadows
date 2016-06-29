#include "Network.h"

SPIFlash flash(FLASH_SS, FLASH_ID); 

void Network::begin(byte nodeID, byte groupID, byte freq, byte powerLevel) {
	Serial << F("Network. startup.") << endl;

	// EEPROM location for radio settings.
	const byte radioConfigLocation = 42;
	if( nodeID == 255 ) {
		this->myNodeID = EEPROM.read(radioConfigLocation);
		Serial << F("Network. read nodeID from EEPROM=") << this->myNodeID << endl;
	} else {
		Serial << F("Network.  writing nodeID to EEPROM=") << nodeID << endl;
		EEPROM.update(radioConfigLocation, nodeID);
		this->myNodeID = nodeID;
	}
	if( this->myNodeID > 200 ) {
		Serial << F("Network. ERROR no nodeID found in EEPROM!") << endl;
		Serial << F("Enter the node number for this node:") << endl;
		while( ! Serial.available() );
		EEPROM.update(radioConfigLocation, Serial.parseInt());
		return( this->begin(nodeID, groupID, freq, powerLevel) );
	}

	radio.initialize(freq, this->myNodeID, groupID);
	radio.setHighPower(); // using RFM69HW board
	radio.promiscuous(true); // so broadcasts are received
	radio.setPowerLevel(powerLevel);
	
	// set the distances to some reasonable default
	this->distance[0] = this->distance[1] = this->distance[2] = HL;
	
	// get my index
	mI = this->myNodeID % 10;
	lI = left(mI);
	rI = right(mI);
	
	// LED usage
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);

	this->state = M_NORMAL; // default to normal operation
	
	Serial << F("Network. startup complete with node number=") << this->myNodeID << endl;
	
}

byte Network::update() {

	// new traffic?
	if( radio.receiveDone() ) {   
		if( radio.DATALEN==sizeof(this->message) ) {
			// read it
			this->message = *(unsigned long*)radio.DATA;  
			if( radio.TARGETID == this->myNodeID && radio.ACKRequested() ) {
				  radio.sendACK();
//				  Serial << this->myNodeID << F(": ACK sent to ") << this->lastRxNodeID << endl;
			}
			return( radio.SENDERID );
		} else if( radio.TARGETID == this->myNodeID ) {
			// being asked to reprogram ourselves by Gateway?
			CheckForWirelessHEX(this->radio, flash);
		} else if( radio.DATALEN==sizeof(systemState) ) {
			this->state = *(systemState*)radio.DATA;
		} else if( radio.SENDERID==PROGRAMMER_NODE ) {
			// we need to wait until the airwaves are clear for 5 seconds.
			Serial << F("Network. Programmer traffic.") << endl;
			this->state = M_PROGRAM;
		}
	}

	// run the reboot commmand in the update cycle
	if( this->state == M_REBOOT ) {
		resetUsingWatchdog(true);
	}

	return( 0 );
}


void Network::send() {
	// put check in to make sure we're not clobbering messages from other transceivers
	this->update();

	if( this->state != M_PROGRAM ) {
		static byte nextID = lI+20;
		while( ! radio.sendWithRetry(this->lI, (const void*)(&this->message), sizeof(this->message), 3, 10)) {
			this->update();
		}
	}
}

void Network::showMessage() {
	Serial << F("Network. MSG");
	Serial << F("\ts=")  << this->s;
	Serial << F("\td0=") << this->distance[0];
	Serial << F("\td1=") << this->distance[1];
	Serial << F("\td2=") << this->distance[2];
	Serial << endl;
}

void Network::encodeMessage() {

	this->message = 0;
	
	this->message |= this->distance[2] & 1023UL;
	this->message = this->message << 10;
//	Serial.println(this->message, BIN);
	
	this->message |= this->distance[1] & 1023UL;
	this->message = this->message << 10;
//	Serial.println(this->message, BIN);
	
	this->message |= this->distance[0] & 1023UL;
	this->message = this->message << 2;
//	Serial.println(this->message, BIN);

	this->message |= this->s & 3UL;
//	Serial.println(this->message, BIN);
	
}

void Network::decodeMessage() {
	this->s = this->message & 3UL; // dumping 30 MSB
	this->distance[0] = (this->message >>  2) & 1023UL; // dumping six MSB
	this->distance[1] = (this->message >> 12) & 1023UL; // dumping six MSB
	this->distance[2] = (this->message >> 22) & 1023UL; // dumping six MSB

	// the distance measures to the right and left of the LED strips and
	// the known distance between the sensors form a triangle.

	// altitude base.  the distance of the triangle's altitude along the side length.
	for(byte i=0; i<3; i++) Ab[i] = altitudeBase(i);
	
	// altitude height.  the extent of the triangle's altitude.
	// these are the trilinear coordinates of the object.
	// https://en.wikipedia.org/wiki/Trilinear_coordinates
	for(byte i=0; i<3; i++) Ah[i] = altitudeHeight(i);
	
	// use Vivani's theorem to adjust the sum of the heights to total height.
//	word deltaAh = (HL - (Ah[lI]+Ah[mI]+Ah[rI])) >> 1; // >>1 is /2.
//	word deltaAh = (HL - (Ah[lI]+Ah[mI]+Ah[rI])) / 2;
	word deltaAh = HL;
	for(byte i=0; i<3; i++) deltaAh-=Ah[i];
	deltaAh = deltaAh < HL ? deltaAh >> 1 : 0; // >>1 is /2
	for(byte i=0; i<3; i++) Ah[i] = Ah[i]==0 ? deltaAh : Ah[i];

	// from here, we want to know the location on the side length that is
	// collinear with the opposite sensor and the object. that is, a 
	// cevian from the opposite vertex that passes through the object.
	// https://en.wikipedia.org/wiki/Cevian
	
	// collinear base. the distance of the collinear point along the side length.
	mCb = collinearBase(mI);
	
	// collinear height. the distance between the collinear point and the object.
	mCh = collinearHeight(mI);
	
	// fractional area. this is the barycentric coordinate relative to the opposite vertex.
	mArea = area(mI);
	
}

// given two distances and a known side length, we can calculate the 
// the altitude's distance along the side length of the altitude.
// from https://en.wikipedia.org/wiki/Heron%27s_formula#Algebraic_proof_using_the_Pythagorean_theorem
word Network::altitudeBase(byte i) {
	// use right-i and left-i distances
	return(
		((SL2+squared(distance[right(i)]))-squared(distance[left(i)])) / twoSL
	);
}

// given the altitude's distance along the side length and the distance,
// use Pythagorean's theorem to calculte the altitude 
word Network::altitudeHeight(byte i) {
	// use right-i distance and i altitude base
	if( Ab[i] > distance[right(i)] ) return( 0 );
	return( 
		squareRoot( squared(distance[right(i)]) - squared(Ab[i]) ) 
	);
}

// given the left and right altitudes to the object, we leverage these
// trilinear coordinates to express the colinear point's location
// on the side length. trilinear coordinates, in particular, enable an 
// algebraic solution involving determinants. otherwise, we'd require
// calculations of angles.
// https://en.wikipedia.org/wiki/Trilinear_coordinates#Collinearities_and_concurrencies
word Network::collinearBase(byte i) {
	// use left-i altitude height and right-i altitude height
	return(
		((unsigned long)(Ah[left(i)])*SL)/((unsigned long)Ah[left(i)]+(unsigned long)Ah[right(i)])
	);
}

// given the extent of the altitude and the difference beween the collinear 
// base intercept and the altitude's base intercept, we can use 
// Pythagorean's theorem to compute the length of the collinear line. 
word Network::collinearHeight(byte i) {
	if( i!=mI ) { Serial << F("need to implement Cb[3] to proceed.") << endl; while(1) update(); }
	// use i altitude height and base; i collinear base
	return( mCb >= Ab[i] ? // unsigned, so careful of order
		squareRoot( squared(Ah[i]) + squared(mCb-Ab[i]) ) :
		squareRoot( squared(Ah[i]) + squared(Ab[i]-mCb) )
	);
}

// finally, we can convert the trilinear coordinates to barycentric coordinates,
// which immediately gives us the FRACTION of the total triangle area
// occupied by the triangle above this LED segment.
// interpret the return of this function to be 65534ths 
// https://en.wikipedia.org/wiki/Barycentric_coordinate_system
word Network::area(byte i) {
	// use i altitude height
	// multiply the ratio of Ah[i]/sum(Ah[]) by the maximum word value to
	// generate a 16-bit fractional result.
	return(
		((unsigned long)((word)(-1))*(unsigned long)Ah[i]) / (unsigned long)SL
	);
}


unsigned long Network::squared(word x) {
	// taking care to promote operand 
	return(
		(unsigned long)x * (unsigned long)x
	);
}

// From: http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
word Network::squareRoot(unsigned long x) {
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

byte Network::right(byte i) {
	// get the tens digit
	byte tens = i - (i % 10); 
	// get the ones digit
	byte ones = i - tens;
	
	return( tens + ones<2 ? ones+1 : 0 );
}

byte Network::left(byte i) {
	// get the tens digit
	byte tens = i - (i % 10); 
	// get the ones digit
	byte ones = i - tens;
	
	return( tens + ones>0 ? ones-1 : 2 );
}

Network N;

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


byte Network::left(byte i) {
	// get the tens digit
	byte tens = i - (i % 10); 
	// get the ones digit
	byte ones = i - tens;
	
	return( tens + ones<2 ? ones+1 : 0 );
}

byte Network::right(byte i) {
	// get the tens digit
	byte tens = i - (i % 10); 
	// get the ones digit
	byte ones = i - tens;
	
	return( tens + ones>0 ? ones-1 : 2 );
}

void Network::showMessage() {
	Serial << F("Network. MSG");
	Serial << F("\ts=")  << this->s;
	Serial << F("\td0=") << this->distance[0];
	Serial << F("\td1=") << this->distance[1];
	Serial << F("\td2=") << this->distance[2];
	Serial << endl;
}

void Network::encodeMessage(word distance) {
	this->distance[mI] = distance;

	this->message = s;
	
	this->message = this->message << 10;
	this->message |= this->distance[2];
	
	this->message = this->message << 10;
	this->message |= this->distance[1];
	
	this->message = this->message << 10;
	this->message |= this->distance[0];
}

void Network::decodeMessage() {
	this->distance[0] = (this->message >>  0) << 6 >> 6; // dumping six MSB
	this->distance[1] = (this->message >> 10) << 6 >> 6; // dumping six MSB
	this->distance[2] = (this->message >> 20) << 6 >> 6; // dumping six MSB
	this->s = (this->message >> 30);

	// altitude base
	Ab[lI] = altitudeBase(lI);
	Ab[mI] = altitudeBase(mI);
	Ab[rI] = altitudeBase(rI);
	
	// altitude height
	Ah[lI] = altitudeHeight(lI);
	Ah[mI] = altitudeHeight(mI);
	Ah[rI] = altitudeHeight(rI);
	
	// use Vivani's theorem to adjust the sum of the heights to total height.
	word deltaAh = (HL - (Ah[lI]+Ah[mI]+Ah[rI])) >> 1; // >>1 is /2.
	Ah[lI] = Ah[lI]==0 ? deltaAh : Ah[lI];
	Ah[mI] = Ah[mI]==0 ? deltaAh : Ah[mI];
	Ah[rI] = Ah[rI]==0 ? deltaAh : Ah[rI];
	
	// collinear base
	mCb = collinearBase(mI);
	
	// collinear height
	mCh = collinearHeight(mI);
	
	// fractional area
	mArea = area(mI);
	
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


word Network::altitudeBase(byte i) {
	// use right-i and left-i distances
	return(
		((SL2+squared(distance[left(i)]))-squared(distance[right(i)])) / twoSL
	);
}
word Network::altitudeHeight(byte i) {
	// use right-i distance and i altitude base
	if( Ab[i] > distance[right(i)] ) return( 0 );
	return( 
		squareRoot( squared(distance[right(i)]) - squared(Ab[i]) ) 
	);
}
word Network::collinearBase(byte i) {
	// use left-i altitude height and right-i altitude height
	return(
		((unsigned long)Ah[left(i)]*SL)/((unsigned long)Ah[left(i)]+(unsigned long)Ah[right(i)])
	);
}
word Network::collinearHeight(byte i) {
	if( i!=mI ) { Serial << F("need to implement Cb[3] to proceed.") << endl; while(1) update(); }
	// use i altitude height and base; i collinear base
	return( mCb >= Ab[i] ? // unsigned, so careful of order
		squareRoot( squared(Ah[i]) + squared(mCb-Ab[i]) ) :
		squareRoot( squared(Ah[i]) + squared(Ab[i]-mCb) )
	);
}
// use FastLED/lib8tion.h lerp16by16() to scale this fraction to integer
word Network::area(byte i) {
	// use i altitude height
	// multiply the ratio of Ah[i]/sum(Ah[]) by the maximum word value to
	// generate a 16-bit fractional result.
	return(
		((unsigned long)((word)(-1))*(unsigned long)Ah[i]) / (unsigned long)SL
	);
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

Network N;

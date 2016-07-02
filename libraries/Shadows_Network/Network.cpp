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
	if( this->myNodeID == 255 ) {
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
	this->distance[0] = this->distance[1] = this->distance[2] = 500;
	
	// get my index
	this->myIndex = this->myNodeID % 10;
	
	// LED usage
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);

	this->state = M_NORMAL; // default to normal operation
	
	Serial << F("Network. startup complete with myNodeID=") << this->myNodeID << F(" myIndex=") << this->myIndex << endl;
	
}


// to my left is my index +1
byte Network::left(byte i) {
	return(
		i % 10 == 2 ? i-2 : i+1
	);	
}

// to my right is my index -1
byte Network::right(byte i) {
	return(
		i % 10 == 0 ? i+2 : i-1
	);	
	
	// get the tens digit
	byte tens = i - (i % 10); 
	// get the ones digit
	byte ones = i % 10;
	
	Serial << "left " << tens << "\t" << ones << "\t" << (tens + ones>0 ? ones-1 : 2) << endl;
	return( tens + ones>0 ? ones-1 : 2 );
}


boolean Network::update() {

	// new traffic?
	if( radio.receiveDone() ) {  
		// check for programming 
		if( radio.SENDERID == PROGRAMMER_NODE && radio.TARGETID == this->myNodeID ) {
			Serial << F("Network. Reprogramming message?") << endl;
			// being asked to reprogram ourselves by Gateway?
			CheckForWirelessHEX(this->radio, flash);
			
		} 

		// check for messages
		if( radio.DATALEN==sizeof(message) ) {
			
			// read it
			message = *(unsigned long*)radio.DATA; 
			targetNodeID = radio.TARGETID;
			senderNodeID = radio.SENDERID;
			
			if( targetNodeID == myNodeID && radio.ACKRequested() ) {
				  radio.sendACK();
//				  Serial << this->myNodeID << F(": ACK sent to ") << this->lastRxNodeID << endl;
			}
			return( true );
			
		} else if( radio.DATALEN==sizeof(systemState) ) {

			// read it
			state = *(systemState*)radio.DATA;
			targetNodeID = radio.TARGETID;
			senderNodeID = radio.SENDERID;

			if( targetNodeID == myNodeID && radio.ACKRequested() ) {
				  radio.sendACK();
//				  Serial << this->myNodeID << F(": ACK sent to ") << this->lastRxNodeID << endl;
			}
			
			// run the reboot commmand right now
			if( state == M_REBOOT ) {
				resetUsingWatchdog(true);
			}
			
			return( true );
			
		} else if( radio.SENDERID == PROGRAMMER_NODE && radio.DATALEN > 30 ) {
			
			// we need to wait until the airwaves are clear.
			Serial << F("Network. Programmer traffic.") << endl;
			state = M_PROGRAM;
		}
	}


	return( false );
}

void Network::showNetwork() {
	Serial << F("Network. ");
	Serial << senderNodeID << F("->") << targetNodeID;
	Serial << F("\ts=")  << s;
	Serial << F("\tmsg=") << _BIN(message);
	Serial << F("\tstate=") << state;
	Serial << endl;
}

/*
message is 32 bits:
	0-9   = d[1]
	10-19 = d[2]
	20-29 = d[3]
	30-31 = s (two extra MSB)

with 10 bits of information, values up to 1023 are possible.  the maximum
sensor reading is ~65 inches, so we could transmit decainches (~650 din)
comfortably.  Importantly, we can use word storage for the decainch data, and cubic 
operations would still fit in unsigned long (32 bits).
*/

void Network::decodeMessage() {
	this->s = this->message & 3UL; // dumping 30 MSB
	this->distance[0] = (this->message >>  2) & 1023UL; // dumping six MSB
	this->distance[1] = (this->message >> 12) & 1023UL; // dumping six MSB
	this->distance[2] = (this->message >> 22) & 1023UL; // dumping six MSB
}

boolean Network::sendState(byte toNodeID) {
	// put check in to make sure we're not clobbering messages from other transceivers
	update();

	targetNodeID = toNodeID;
	senderNodeID = myNodeID;

	if( toNodeID != BROADCAST ) {
		return( 
			radio.sendWithRetry(toNodeID, (const void*)(&state), sizeof(state), 3, 10)
		);
	} else {
		radio.send(BROADCAST, (const void*)(&state), sizeof(state));
		return( true );
	}
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


boolean Network::sendMessage(byte toNodeID) {
	// put check in to make sure we're not clobbering messages from other transceivers
	update();

	targetNodeID = toNodeID;
	senderNodeID = myNodeID;

	if( toNodeID != BROADCAST ) {
		return( 
			radio.sendWithRetry(toNodeID, (const void*)(&message), sizeof(message), 3, 10)
		);
	} else {
		radio.send(BROADCAST, (const void*)(&message), sizeof(message));
		return( true );
	}

}

Network N;

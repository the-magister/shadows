#include "Network.h"

SPIFlash flash(FLASH_SS, FLASH_ID); 

void Network::begin(byte groupID, byte freq, byte powerLevel) {
	Serial << F("Network. startup.") << endl;

	// EEPROM location for radio settings.
	const byte radioConfigLocation = 42;
	this->myNodeID = EEPROM.read(radioConfigLocation);
	Serial << F("Network. read nodeID from EEPROM=") << this->myNodeID << endl;

	if( this->myNodeID > 25 ) {
		Serial << F("Network. ERROR no nodeID found in EEPROM!") << endl;
		Serial << F("Enter the node number for this node:") << endl;
		while( ! Serial.available() );
		EEPROM.write(radioConfigLocation, Serial.parseInt());
		return( this->begin(groupID, freq, powerLevel) );
	}

	radio.initialize(freq, this->myNodeID, groupID);
	radio.setHighPower(); // using RFM69HW board
	radio.promiscuous(true); // so broadcasts are received
	radio.setPowerLevel(powerLevel);
	
	this->inStartup = true; // bootstrap
	
	msg.d[0] = msg.d[1] = msg.d[2] = 255;
	msg.inter[0] = msg.inter[1] = msg.inter[2] = 255;
	msg.range[0] = msg.range[1] = msg.range[2] = 255;
	
	this->lastRxNodeID = 200; // bootstrap to first transceiver
	
	pinMode(LED, OUTPUT);

	Serial << F("Network. startup complete with node number=") << this->myNodeID << endl;
}

byte Network::whoAmI() {
	return( this->myNodeID );
}

boolean Network::update() {
	// new traffic?
	if( radio.receiveDone() ) {   
		if( radio.DATALEN==sizeof(Message) ) {
			// read it
			this->msg = *(Message*)radio.DATA;  
			this->lastRxNodeID = radio.SENDERID;
			return( true );
		} else {
			// and we might be asked to reprogram ourselves by Gateway
			CheckForWirelessHEX(this->radio, flash, true);
		}
	}

	// are we trying to bootstrap?
	if( this->inStartup ) {
		// set the last nodeID received to be outside of the range.
		// the first transmitter will then be the next to transmit.
		this->inStartup = false;
		this->lastRxNodeID = 200;
		return( true );
	}
	
	return( false );
}

void Network::printMessage() {
	Serial << F("Network. MSG:");
	Serial << F("\td 0=") << msg.d[0] << F(" 1=") << msg.d[1] << F(" 2=") << msg.d[2];
	Serial << F("\ti 0=") << msg.inter[0] << F(" 1=") << msg.inter[1] << F(" 2=") << msg.inter[2];
	Serial << F("\tr 0=") << msg.range[0] << F(" 1=") << msg.range[1] << F(" 2=") << msg.range[2];
	Serial << endl;
}

byte Network::isNext(byte node, byte maxNode, byte minNode) {
  // skipping modulo arithmetic to minimize instructions
  return( (node+1) >= (maxNode+1) ? minNode : node+1 );
}

byte Network::isPrev(byte node, byte maxNode, byte minNode) {
  return( (node-1) <= (minNode-1) ? maxNode : node-1 );
}

boolean Network::meNext() {
  return( isNext(this->lastRxNodeID, 12, 10) == this->myNodeID );
}

void Network::send() {
	radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));
}

byte Network::distance() {
  return( this->msg.d[this->myNodeID-20] );
}

byte Network::range() {
  return( this->msg.range[this->myNodeID-20] );
}

byte Network::intercept() {
  return( this->msg.inter[this->myNodeID-20] );
}

boolean Network::objectAnywhere() {
  return( msg.d[0] < 254 || msg.d[1] < 254 || msg.d[2] < 254 );
}

boolean Network::objectInPlane() {
  return( msg.d[0] <= BASE_LEN && msg.d[1] <= BASE_LEN && msg.d[2] <= BASE_LEN );
}


Network N;

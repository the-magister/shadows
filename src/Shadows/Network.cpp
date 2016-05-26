#include "Network.h"

SPIFlash flash(FLASH_SS, FLASH_ID); 

void Network::begin(byte nodeID, byte groupID, byte freq, byte powerLevel) {
	Serial << F("Radio. startup node number=") << nodeID << endl;

	// EEPROM location for radio settings.
	const byte radioConfigLocation = 42;

	if( nodeID == BROADCAST ) {
		this->myNodeID = EEPROM.read(radioConfigLocation);
		Serial << F("Radio. read nodeID from EEPROM=") << this->myNodeID << endl;
	} else {
		this->myNodeID = nodeID;
		EEPROM.write(radioConfigLocation, nodeID);
		Serial << F("Radio. set nodeID to EEPROM=") << this->myNodeID << endl;
	}

	radio.initialize(freq, this->myNodeID, groupID);
	radio.setHighPower(); // using RFM69HW board
	radio.promiscuous(true); // so broadcasts are received
	radio.setPowerLevel(powerLevel);
	
	this->inStartup = true; // bootstrap
	
	Serial << F("Radio. startup complete with node number=") << this->myNodeID << endl;

  pinMode(LED, OUTPUT);
}

byte Network::whoAmI() {
	return( this->myNodeID );
}

boolean Network::update() {
	// new traffic?
	if( radio.receiveDone() && radio.DATALEN==sizeof(Message) ) {   
		if( radio.DATALEN==sizeof(Message) ) {
			// read it
			msg = *(Message*)radio.DATA;  
			return( true );
		} else {
			// and we might be asked to reprogram ourselves by GateWay
			CheckForWirelessHEX(this->radio, flash, true);
		}
		
	}

	// are we trying to bootstrap?
	if( this->inStartup ) {
		// set the nodeID received to be outside of the range.
		// the first transmitter will then be the next to transmit
		this->inStartup = false;
		this->msg.nodeID = 200;
		this->msg.distance = 9999; 
		return( true );
	}
	
	return( false );
}

byte Network::lastDistanceNode() {
	return( this->msg.nodeID );
}

long Network::lastDistanceValue() {
	return( this->msg.distance );
}

void Network::sendDistance(long distance) {
	this->msg.nodeID = this->myNodeID;
	this->msg.distance = distance;
	
	radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));
}

Network N;

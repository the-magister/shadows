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
	
	Serial << F("Radio. startup complete with node number=") << this->myNodeID << endl;

  pinMode(LED, OUTPUT);
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

byte Network::isNext(byte node, byte maxNode, byte minNode) {
  // skipping modulo arithmetic to minimize instructions
  return( (node+1) == (maxNode+1) ? minNode : node+1 );
}

byte Network::isPrev(byte node, byte maxNode, byte minNode) {
  return( (node-1) == (minNode-1) ? maxNode : node-1 );
}

boolean Network::meNext() {
  return( isNext(this->lastRxNodeID, 12, 10) == this->myNodeID );
}

void Network::send() {
	radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));
}

byte Network::posX() {
  return( this->msg.x[this->myNodeID-20] );
}

byte Network::posY() {
  return( this->msg.y[this->myNodeID-20] );
}

boolean Network::isObject(byte xMax, byte yMax) {
  if( this->posX() <= xMax && this->posY() <= yMax ) {
    return( true );
  } else {
    return( false );
  }
}



Network N;

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
		EEPROM.write(radioConfigLocation, nodeID);
		this->myNodeID = nodeID;
	}
	if( this->myNodeID > 200 ) {
		Serial << F("Network. ERROR no nodeID found in EEPROM!") << endl;
		Serial << F("Enter the node number for this node:") << endl;
		while( ! Serial.available() );
		EEPROM.write(radioConfigLocation, Serial.parseInt());
		return( this->begin(nodeID, groupID, freq, powerLevel) );
	}

	radio.initialize(freq, this->myNodeID, groupID);
	radio.setHighPower(); // using RFM69HW board
	radio.promiscuous(true); // so broadcasts are received
	radio.setPowerLevel(powerLevel);
	
	msg.d[0] = msg.d[1] = msg.d[2] = D_OFFLINE;
	msg.inter[0] = msg.inter[1] = msg.inter[2] = P_OFFLINE;
	msg.range[0] = msg.range[1] = msg.range[2] = P_OFFLINE;
	
	this->lastRxNodeID = 12; // bootstrap to first transceiver
	
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);

	Serial << F("Network. bitrate(bits/s)=55555.") << endl;
	
	// packet contents: http://lowpowerlab.com/blog/2013/06/20/rfm69-library/
	byte packetSizebits = 8*(3+2+4+sizeof(Message)+2);
	
	Serial << F("Network. packet size(bits)=") << packetSizebits << F(" (bytes)=") << packetSizebits/8 << endl;
	
	Serial << F("Network. approximate packet transmission time(ms)=") << packetSizebits / 56 + 1 << endl;
	
	Serial << F("Network. startup complete with node number=") << this->myNodeID << endl;
	
}

byte Network::whoAmI() {
	return( this->myNodeID );
}

boolean Network::update() {
	// new traffic?
	if( radio.receiveDone() ) {   
		Serial << F("Network. radio RX") << endl;
		if( radio.DATALEN==sizeof(Message) ) {
			// read it
			this->msg = *(Message*)radio.DATA;  
			this->lastRxNodeID = radio.SENDERID;
			Serial << F("Network. message RX") << endl;
			return( true );
		} else {
			// and we might be asked to reprogram ourselves by Gateway
			CheckForWirelessHEX(this->radio, flash, true);
		}
	}

	return( false );
}

void Network::printMessage() {
	Serial << F("Network. MSG from ") << this->lastRxNodeID;
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

boolean Network::meLast() {
  return( this->lastRxNodeID == this->myNodeID );
}

void Network::send() {
	Serial << F("Network. send.") << endl;
	radio.send(BROADCAST, (const void*)(&this->msg), sizeof(Message));
	this->lastRxNodeID = this->myNodeID;
}

word Network::distance() {
  return( this->msg.d[this->myNodeID-20] );
}

word Network::range() {
  return( this->msg.range[this->myNodeID-20] );
}

word Network::intercept() {
  return( this->msg.inter[this->myNodeID-20] );
}

boolean Network::objectAnywhere() {
  return( msg.d[0] < 25400 || msg.d[1] < 25400 || msg.d[2] < 25400 );
}

boolean Network::objectInPlane() {
  return( msg.d[0] <= BASE_LEN && msg.d[1] <= BASE_LEN && msg.d[2] <= BASE_LEN );
}


Network N;

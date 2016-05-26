#include "Identity.h"

void Identity::begin(byte Iam) {
  Serial << F("Identity. startup with node=") << Iam << endl;

	this->myNodeID = Iam;
	
	if( Iam >= 20 ) {
		this->IamTransmitter = false;
		this->IamController = true;
		Serial << F("Identity. I am transmitter.") << endl;
	} else {
		this->IamTransmitter = true;
		this->IamController = false;
		Serial << F("Identity. I am controller.") << endl;
	}
}

byte Identity::isNext(byte node, byte maxNode, byte minNode) {
	// skipping modulo arithmetic to minimize instructions
	return( (node+1) == (maxNode+1) ? minNode : node+1 );
}

byte Identity::isPrev(byte node, byte maxNode, byte minNode) {
	return( (node-1) == (minNode-1) ? maxNode : node-1 );
}

boolean Identity::amTransmitter() { 
	return( this->IamTransmitter ); 
}

boolean Identity::meNext(byte node) {
	// if I'm not a transmitter, I can't be next
	if( ! this->IamTransmitter ) return( false );
	
	return( isNext(node, 12, 10) == this->myNodeID );
}

boolean Identity::amController() { 
	return( this->IamController ); 
}

boolean Identity::myAcross(byte node) {
	// if I'm not a controller, this can't be true
	if( ! this->IamController ) return( false );
	
	return( (node)+10 == this->myNodeID );
}

boolean Identity::myLeft(byte node) {
	// if I'm not a controller, this can't be true
	if( ! this->IamController ) return( false );
	
	return( isNext(node, 12, 10)+10 == this->myNodeID );
}

boolean Identity::myRight(byte node) {
	// if I'm not a controller, this can't be true
	if( ! this->IamController ) return( false );
	
	return( isPrev(node, 12, 10)+10 == this->myNodeID );
}

Identity I;

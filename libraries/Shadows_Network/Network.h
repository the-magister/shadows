#ifndef Network_h
#define Network_h

#include <Arduino.h>

#include <Streaming.h>

// radio
#include <RFM69.h> 
#include <SPI.h> 
// wireless programming
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
// for storage of node information
#include <EEPROM.h> 

// comms settings and information

#define BROADCAST			0  // all nodes will hear this
#define GROUPID				157  // local group
#define POWERLEVEL			31 // 0-31, 31 being maximal

#define PROGRAMMER_NODE			254 // nodeID of wireless programmer

#define N_NODES				3 // 3 Lights, 3 Location nodes

// system state messages
enum systemState {
	M_CALIBRATE=0,	// calibration of sensors
	M_NORMAL,		// normal operation
	M_PROGRAM,		// OTA programming
	M_REBOOT, 		// reboot the nodes
	
	N_MODES 		// track mode count
};

// pin definitions common to Moteuinos
#define LED			9 // Moteinos have LED on D9
#define FLASH_SS	8 // and FLASH SS on D8
#define FLASH_ID	0xEF30 // EF30 for windbond 4mbit flash

class Network {
  public:
	// initialize radio
	void begin(byte nodeID=255, byte groupID=GROUPID, byte freq=RF69_915MHZ, byte powerLevel=POWERLEVEL);
	// return my node ID
	byte myNodeID; // 10-12 for Location; 20-22 for Lights
	// return my index into arrays
	byte myIndex; // 0-2
	// which can be used to understand what is to the right and left
	byte right(byte i);
	byte left(byte i);

	// check for radio traffic; return true if we have a message or state change
	boolean update();
	byte senderNodeID, targetNodeID; // from and to information for message
	unsigned long message; // message 
	systemState state; // system state
	
	// show the contents of the network information
	void showNetwork();

	// translate message -> distance
	void decodeMessage();
	word distance[N_NODES]; // distances from sensors to object
	byte s; // 2 extra bits in message

	// send state; returns true if ACKd or toNodeID==BROADCAST
//	boolean sendState(byte toNodeID=BROADCAST);
	void sendState(byte toNodeID=BROADCAST);

	// for Node_Location
	// translate distance -> message
	void encodeMessage();
	// send messsage; returns true if ACKd or toNodeID==BROADCAST
//	boolean sendMessage(byte toNodeID);
	void sendMessage(byte toNodeID);

	// all of this is conducted over radio
	RFM69 radio;
	
};

extern Network N;

#endif

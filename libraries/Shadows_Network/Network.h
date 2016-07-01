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

#define PROGRAMMER_NODE		254 // detecting traffic from this node should tell everyone to STFU already

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

// geometry of the devices, 16-bit
#define SL			755U // sensor-sensor distance; i.e. side length
#define HL			655U // sensor-LED distance; i.e. altitude
#define IN_PLANE	625U // sensor-LED distance threshold for "detected something"
#define LL			720U // LED strip length; 

// useful constants, 32 bit
#define SL2			(755UL*755UL) // squared side length
#define twoSL		(2UL*755UL)   // two times size length

/*
Physical layout:
    ---- SL --->
         20
   11 -------- 12   ^
     \        /     |
      \      /      HL
    22 \    / 21    |
        \  /        |
         10         |

Tens digit:
  1 = nodes with ultrasound rangefinders (Node_Location)
  2 = nodes with RGB LED strips (Node_Light)
  
Ones digit (with same tens digit):
  +1 (modulo) = "to my right"/next
  -1 (modulo) = "to my left"/previous
  
Ultrasound ring goes clockwise round-robin:

  Rx From	Is Next		Tx To
  10		11			12
  11		12			10
  12		10			11
  
*/ 

// NOTE: I specifically skipped getter/setter functions to 
// strip the memory usage down to be as small as possible.
// Stylistically, this is ugly, and it does exposure private 
// components to unintended alteration.  However, both the 
// sensor and lighting nodes use this code so I want a very
// small memory footprint. I did use some bytes for indexing
// variables, as I felt that the readibility of the code
// was greatly improved, so this is a compromise position.

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

	// for both Node_Light and Node_Location
	// check for radio traffic; return true if we have a message
	boolean update();
	byte senderNodeID;
	unsigned long message;
	// show the contents of the message information
	void showMessage();

	// translate message -> distance
	void decodeMessage();
	word distance[N_NODES]; // distance from sensor to object
	byte s; // 2 extra bits in message

	// system state
	systemState state;

	// for Node_Location
	// translate distance -> message
	void encodeMessage();
	// send messsage 
	void send(byte toNodeID);

	// for Node_Lights
	void calculateLocation();
	// which sets the following target information
	word Ab[N_NODES], Ah[N_NODES]; // object location relative to LEDs, altitude basis
	word mCb[N_NODES], mCh[N_NODES]; // object location relative to LEDs, collinear basis
	word mArea[N_NODES]; // relative area of the triangle defined by the object and LEDs, relative to total area.
	
	// all of this is conducted over radio
	RFM69 radio;
	
  private:

	// helper functions
	unsigned long squared(word x);
	word squareRoot(unsigned long x);

	word altitudeBase(byte i);
	word altitudeHeight(byte i);
	word collinearBase(byte i);
	word collinearHeight(byte i);
	word area(byte i);
};

extern Network N;

#endif

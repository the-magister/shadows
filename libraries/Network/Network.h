#ifndef Network_h
#define Network_h

#include <Arduino.h>

#include <Metro.h>
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

#define BROADCAST			 0  // all nodes will hear this
#define GROUPID				157  // local group
#define POWERLEVEL			31 // 0-31, 31 being maximal

#define PROGRAMMER_NODE		254 // detecting traffic from this node should tell everyone to STFU already

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

// geometry of the devices
#define SL			755U // sensor-sensor distance; i.e. side length
#define HL			655U // sensor-LED distance; i.e. altitude
#define IN_PLANE	625U // sensor-LED distance threshold for "detected something"
#define LL			720U // LED strip length; 

// useful constants
#define SL2			755UL*755UL // squared side length
#define twoSL		2UL*755UL   // two times size length

/*
Physical layout:
    ---- SL --->
         20
   12 -------- 11   ^
     \        /     |
      \      /      HL
    21 \    / 22    |
        \  /        |
         10         |

Tens digit:
  1 = nodes with ultrasound rangefinders (Node_Location)
  2 = nodes with RGB LED strips (Node_Light)
  
Ones digit (with same tens digit):
  +1 (modulo) = "to my right"/next
  -1 (modulo) = "to my left"/previous
  
Ultrasound ring goes counter-clockwise round-robin:

  Rx From   Is Next
  10		11
  11		12
  12		10
  
*/ 


/*
message is 32 bits:
	0-9   = d[1]
	10-19 = d[2]
	20-29 = d[3]
	30-31 = enum { OOR_3, OOR_2, OOR_1, OK } indicating sensor ranging outcome

that would turn the message size from 2*3*3=18 bytes to 4*1=4 bytes, which
would cut the transmission time from ~4ms to ~2ms.  With the ACK, that's ~8ms to ~4ms,
which is significant in the context of a ~10ms ranging activity.  

with 10 bits of information, values up to 1023 are possible.  the maximum
sensor reading is ~65 inches, so we could transmit decainches (~650 din)
comfortably.  Importantly, we can use word storage for the decainch data, and cubic 
operations would still fit in unsigned long (32 bits).
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
	byte myNodeID;
	// indexing information; left, me, right
	byte lI, mI, rI;
	// or, a more general form
	byte right(byte i);
	byte left(byte i);

	// for both Node_Light and Node_Location
	// check for radio traffic; return sender's node ID if we have a Message
	byte update();
	unsigned long message;

	// show the contents of the message information
	void showMessage();
	
	// system state
	systemState state;

	// for Node_Location
	void encodeMessage(word distance);
	// send location information encoded in msg
	void send();

	// for Node_Lights
	void decodeMessage();
	// which sets the following target information
	byte s; // 2 MSBs in message
	word distance[3]; // distance from sensor to object
	word Ab[3], Ah[3]; // object location relative to LEDs, altitude basis
	word mCb, mCh; // object location relative to this LED, collinear basis
	word mArea; // relative area of the triangle defined by the object and this LED, relative to total area.
	
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

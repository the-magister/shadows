#ifndef Network_h
#define Network_h

#include <Arduino.h>

#include <Metro.h>
#include <Streaming.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>

#include <EEPROM.h> // for storage of node information

#define BROADCAST 0  // all nodes will hear this
#define GROUPID 157  // local group
#define POWERLEVEL 31 // 0-31, 31 being maximal

#define PROGRAMMER_NODE 254 // detecting traffic from this node should tell everyone to STFU already

typedef struct {
  word d[3];      // distance relative to sensors; pull by d[this->myNodeID-10]
  word inter[3];  // projected intersection on lights; pull by x[this->myNodeID-20]
  word range[3];  // projected distnace lights; pull by y[this->myNodeID-20]
} Message;

enum systemState {
	M_CALIBRATE=0,	// calibration of sensors
	M_NORMAL,		// normal operation
	M_PROGRAM,		// OTA programming
	M_REBOOT, 		// reboot the nodes
	
	N_MODES 		// track mode count
};

// pin definitions common to Moteuinos
#define LED           9 // Moteinos have LED on D9
#define FLASH_SS      8 // and FLASH SS on D8
#define FLASH_ID      0xEF30 // EF30 for windbond 4mbit flash

// geometry of the devices
#define BASE_LEN      7200U // length of LED strips (centainches)
#define HALF_BASE     3600U // halfway along the LED strip (centainches)

#define SENSOR_DIST   7550U // distance between sensors

//#define HEIGHT_LEN    6235U // height of the sensor over the LEDs (centainches)
#define HEIGHT_LEN    6550U // height of the sensor over the LEDs (centainches)

#define IN_CORNER     1200U   // any sensor distance closer than this indicates the object is cornered.
#define IN_PLANE      HEIGHT_LEN-1000U    // any sensor distance less than this indicates an object in plane

// range definitions
#define D_OFFLINE     65535U  // one or more sensors haven't reported in
#define D_ERROR       65534U  // distance information is in error

// position definition
#define P_OFFLINE     65535U  // one or more sensors haven't reported in
#define P_ERROR       65534U  // distance information can't be used to triangulate locations

/*
Physical layout:

         20
   12 -------- 11
     \        /
      \      / 
    21 \    / 22
        \  /
         10

Tens digit:
  1 = nodes with ultrasound rangefinders (Node_Location)
  2 = nodes with RGB LED strips (Node_Light)
  
Ones digit (with same tens digit):
  +1 (modulo) = "to my right"/next
  -1 (modulo) = "to my left"/previous
  
Ultrasound ring goes counter-clockwise round-robin:

  Rx From   Is Next
  10        11
  11        12
  12        10
  
*/ 

class Network {
  public:
    // initialize radio
    void begin(byte nodeID=255, byte groupID=GROUPID, byte freq=RF69_915MHZ, byte powerLevel=POWERLEVEL);
    // return my node ID
    byte whoAmI();

    // for both Node_Light and Node_Location
    // check for radio traffic; return true if we have a Message
    boolean update();
    // make the location message available for direct update by Node_Location
    Message msg;
    // show the contents of the message
    void printMessage();
    // set the system state
    void setState(systemState state);
    // get the system state
    systemState getState();

    // for Node_Location
    // am I next to transmit distance information?
    boolean meNext();
	// was I the last to transmit distance information?
	boolean meLast();
    // send location information encoded in msg
    void send();

    // for Node_Lights
    // is there an object in the plane?
    boolean objectInPlane();
    // return the positional information, relative to me
	word myDistance();
    word myIntercept();
    word myRange();
    
  private:
    systemState currentState;

    byte myNodeID, lastRxNodeID;
    
    // radio instance
    RFM69 radio;

    // helper functions
    byte isNext(byte node, byte maxNode, byte minNode);
    byte isPrev(byte node, byte maxNode, byte minNode);
};

extern Network N;

#endif

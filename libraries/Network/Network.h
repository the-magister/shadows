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

typedef struct {
  byte d[3];  // distance relative to sensors; pull by d[this->myNodeID-10]
  byte inter[3];  // projected intersection on lights; pull by x[this->myNodeID-20]
  byte range[3];  // projected distnace lights; pull by y[this->myNodeID-20]
} Message;

#define LED           9 // Moteinos have LED on D9
#define FLASH_SS      8 // and FLASH SS on D8
#define FLASH_ID      0xEF30 // EF30 for windbond 4mbit flash

#define BASE_LEN      72 // length of LED strips
#define HEIGHT_LEN    63 // height of the sensor over the LEDS

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
    // initialize led strips
    void begin(byte groupID=GROUPID, byte freq=RF69_915MHZ, byte powerLevel=POWERLEVEL);
    // return my node ID
    byte whoAmI();

    // for both Node_Light and Node_Location
    // check for radio traffic; return true if we have an update
    boolean update();
    // make the location message available for direct update by Node_Location
    Message msg;
	// show the contents of the message
	void printMessage();

    // for Node_Location
    // am I next to transmit distance information?
    boolean meNext();
    // send location information
    void send();

    // for Node_Lights
	// is there an object outside the plane?
	boolean objectAnywhere();
    // is there an object in the plane?
    boolean objectInPlane();
    // return the positional information, relative to me
    byte intercept();
    byte range();
    
  private:
    boolean inStartup;

    byte myNodeID, lastRxNodeID;
    
    // radio instance
    RFM69 radio;

    // helper functions
    byte isNext(byte node, byte maxNode, byte minNode);
    byte isPrev(byte node, byte maxNode, byte minNode);
};

extern Network N;

#endif

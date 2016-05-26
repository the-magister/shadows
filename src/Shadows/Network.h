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
  byte nodeID;
  long distance;
} Message;

#define LED           9 // Moteinos hsave LEDs on D9
#define FLASH_SS      8 // and FLASH SS on D8
#define FLASH_ID      0xEF30 // EF30 for windbond 4mbit flash

class Network {
  public:
    // initialize led strips
    void begin(byte nodeID = BROADCAST, byte groupID = GROUPID, byte freq = RF69_915MHZ, byte powerLevel = POWERLEVEL);
    // return my node ID
    byte whoAmI();

    // check for radio traffic; return true if we have an update
    boolean update();
    // return last distance nodeID
    byte lastDistanceNode();
    long lastDistanceValue();

    // send distance information
    void sendDistance(long distance);

  private:
    boolean inStartup;

    byte myNodeID;
    // radio instance
    RFM69 radio;
    // message storage
    Message msg;

};

extern Network N;

#endif

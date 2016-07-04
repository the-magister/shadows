// **********************************************************************************
// This sketch is an example of how wireless programming can be achieved with a Moteino
// that was loaded with a custom 1k bootloader (DualOptiboot) that is capable of loading
// a new sketch from an external SPI flash chip
// This is the GATEWAY node, it does not need a custom Optiboot nor any external FLASH memory chip
// (ONLY the target node will need those)
// The sketch includes logic to receive the new sketch from the serial port (from a host computer) and
// transmit it wirelessly to the target node
// The handshake protocol that receives the sketch from the serial port
// is handled by the SPIFLash/WirelessHEX69 library, which also relies on the RFM69 library
// These libraries and custom 1k Optiboot bootloader for the target node are at: http://github.com/lowpowerlab
// **********************************************************************************
// Copyright Felix Rusu, LowPowerLab.com
// Library and code by Felix Rusu - felix@lowpowerlab.com
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************
#include <RFM69.h>          //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h>      //get it here: https://www.github.com/lowpowerlab/spiflash
#include <WirelessHEX69.h> //get it here: https://github.com/LowPowerLab/WirelessProgramming/tree/master/WirelessHEX69
#include <Streaming.h>
#include <Metro.h>
#include <EEPROM.h>
#include <Network.h>

#define ACK_TIME    100  // # of ms to wait for an ack
#define TIMEOUT     3000

char c = 0;
char input[64]; //serial input buffer
byte targetID = 0;

// messages
systemState messageCalibrate = M_CALIBRATE;
systemState messageNormal = M_NORMAL;
systemState messageProgram = M_PROGRAM;
systemState messageReboot = M_REBOOT;

// sniffing and spoofing
boolean sniff = false;
boolean distance = false;

void setup() {
  Serial.begin(115200);

  N.begin(PROGRAMMER_NODE);
 
  Serial.println("Start wireless gateway...");
}

void loop() {
  byte inputLen = 0;
  if( Serial.available() > 0 ) 
    inputLen = readSerialLine(input, 10, 64, 100); //readSerialLine(char* input, char endOfLineChar=10, byte maxLength=64, uint16_t timeout=1000);

  if (inputLen == 4 && input[0] == 'F' && input[1] == 'L' && input[2] == 'X' && input[3] == '?') {
    if (targetID == 0) {
      Serial.println("TO?");
    } else {
      // and let the nodes know a programming string is coming
      sendMessage(messageProgram);
      
      boolean success = CheckForSerialHEX((byte*)input, inputLen, N.radio, targetID, TIMEOUT, ACK_TIME, false);
      if( success ) sendMessage(messageReboot);
      
    }
  } else if (inputLen > 3 && inputLen <= 6 && input[0] == 'T' && input[1] == 'O' && input[2] == ':')  {
    byte newTarget = 0;
    for (byte i = 3; i < inputLen; i++) //up to 3 characters for target ID
      if (input[i] >= 48 && input[i] <= 57)
        newTarget = newTarget * 10 + input[i] - 48;
      else {
        newTarget = 0;
        break;
      }
    if (newTarget > 0) {
      targetID = newTarget;
      Serial.print("TO:");
      Serial.print(newTarget);
      Serial.println(":OK");
      // and let the nodes know a programming string is coming
      sendMessage(messageProgram);
      
    } else {
      Serial.print(input);
      Serial.print(":INV");
    }
  } else if (inputLen == 2 && input[0] == 'S' && input[1] == 'N') {
    sniff = ! sniff;
    Serial << F("Sniffing: ") << sniff << endl;
  } else if (inputLen == 2 && input[0] == 'D' && input[1] == 'I') {
    distance = ! distance;
    Serial << F("Distance: ") << distance << endl;
  } else if (inputLen == 2 && input[0] == 'C' && input[1] == 'A') {
    sendMessage(messageCalibrate);   
    Serial << F("[CA]libration operation Message") << endl;
    delay(1000);
    sendMessage(messageNormal);   
    Serial << F("[NO]ormal operation Message") << endl;
  } else if (inputLen == 2 && input[0] == 'N' && input[1] == 'O') {
    sendMessage(messageNormal);   
    Serial << F("[NO]ormal operation Message") << endl;
  } else if (inputLen == 2 && input[0] == 'P' && input[1] == 'R') {
    sendMessage(messageProgram);
    Serial << F("[PR]rogramming operation Message") << endl;
  } else if (inputLen == 2 && input[0] == 'R' && input[1] == 'E') {
    sendMessage(messageReboot);
    Serial << F("[RE]boot programming Message") << endl;
  } else if (inputLen > 0) { //just echo back and provide instructions
    Serial.print("SERIAL IN > "); Serial.println(input);
    Serial << F("[SN]iff packet contents (toggles)") << endl;
    Serial << F("[DI]istance information in packets (toggles)") << endl;
    Serial << F("[CA]libration operation Message") << endl;
    Serial << F("[NO]ormal operation Message") << endl;
    Serial << F("[PR]rogramming operation Message") << endl;
    Serial << F("[RE]boot programming Message") << endl;
  }

  if( N.update() ) {
    
    N.decodeMessage();
    
    if( sniff ) N.showNetwork();
    
    if( distance ) {
      Serial << F("Distance:\td0=") << N.distance[0] << F("\td1=") << N.distance[1] << F("\td2=") << N.distance[2] << endl;
    }
    
    static unsigned long cycleTime = 50UL;
    
    if( N.senderNodeID == 10 ) {
      static unsigned long then = millis();
      unsigned long now = millis();
      cycleTime = (9*cycleTime + now-then)/10;
      then = now;
    }

    static byte loopCount = 0;
    if( loopCount++ >= 100 ) {
      Serial << F("Cycle time (ms)=") << cycleTime;
      Serial << F("\tFrequency (Hz)=") << 1.0/((float)cycleTime/1000.0) << endl;      
      loopCount = 0;
    }

    Blink(LED, 5); //heartbeat
    
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

void sendMessage(systemState msg) {
  N.state = msg;
  for(byte i=0;i<10; i++) {
    N.sendState();
    delay(5);
  }  
}


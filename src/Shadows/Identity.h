#ifndef Identity_h
#define Identity_h

#include <Arduino.h>
#include <EEPROM.h>
#include <Streaming.h>

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
  1 = nodes with ultrasound rangefinders (inputs)
  2 = nodes with RGB LED strips (output)
  
Ones digit (with same tens digit):
  +1 (modulo) = "to my right"/next
  -1 (modulo) = "to my left"/previous
  
Ultrasound ring:

  Rx From	Is Next
  10		11
  11		12
  12		10
  
  byte isNext(byte RxFrom) { return( ( RxFrom+1 == (12+1) ? 10 : RxFrom+1 ) ); }
  byte isPrev(byte RxFrom) { return( ( RxFrom-1 == (10-1) ? 12 : RxFrom-1 ) ); }
  
Sensor ring
  
  I am		My left		My across	My right
  20		11			10			12
  21		12			11			10
  22		10			12			11
  
  byte myAcross(byte Iam) { return( Iam-10 ); }
  byte myLeft(byte Iam) { return( isNext(myAcross(Iam)) ); }
  byte myRight(byte Iam) { return( isPrev(myAcross(Iam)) ); }
  
*/ 

class Identity {
  public:
    // set my node number
    void begin(byte Iam);

	// am I a transmitter?
	boolean amTransmitter();
	// am I next to transmit distance information?
	boolean meNext(byte node);

	// am I a LED controller?
	boolean amController();
	// was the distance information received from the transmitter across from me?
	boolean myAcross(byte node);
	// was the distance information received from the transmitter left from me?
	boolean myLeft(byte node);
	// was the distance information received from the transmitter left from me?
	boolean myRight(byte node);

  private:
  
	byte myNodeID;
	boolean IamTransmitter, IamController;
	
	byte isNext(byte node, byte maxNode, byte minNode);
	byte isPrev(byte node, byte maxNode, byte minNode);
};

extern Identity I;

#endif

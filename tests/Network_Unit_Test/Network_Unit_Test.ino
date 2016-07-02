#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <Network.h>



unsigned long elapsedTime() {
  static unsigned long then = micros();
  unsigned long now = micros();
  unsigned long delta = now - then;
  then = now;
  return ( delta );
}

boolean compare3(word soln[N_NODES], word calc[N_NODES]) {
  Serial << F("soln\t") << F("\t") << soln[0]  << F("\t") << soln[1]  << F("\t") << soln[2] << endl;
  Serial << F("calc\t") << F("\t") << calc[0]  << F("\t") << calc[1]  << F("\t") << calc[2] << endl;
 
  if ( memcmp(soln, calc, sizeof(calc)) != 0 ) {
    return( false );
  } else {
    return( true );
  }
}

boolean compare(word soln, word calc) {
  Serial << F("soln\t") << F("\t") << soln << endl;
  Serial << F("calc\t") << F("\t") << calc << endl;
 
  if ( soln != calc ) {
    return( false );
  } else {
    return( true );
  }
}


void setup() {

  Serial.begin(115200);

  // start the radio
  N.begin();

  randomSeed(analogRead(4));

  test_s(); // should pass with i=[0,3]
  test_d(0, 1023); // should all pass
  test_d(1024, 65534); // should all fail

  test_state(); // should pass, but requires programmer node to be online

  test_message(); // should pass, but requires programmer node to be online

  test_packets(); // check for packet loss, but requires programer node to be online

  Serial << F("====\nTests complete") << endl;
}

void loop() {

}

// make sure we can correctly encode and decode s
void test_s() {
  Serial << F("==== TEST: encoding and decoding s") << endl;
  for (byte i = 0; i <= 5; i++) {
    N.s = i;
    Serial << F("SOLN: ");
    N.showNetwork();

    // encode, decode
    N.encodeMessage();
    N.decodeMessage();
    
    if ( ! compare(N.s, i ) ) {
      Serial << F("FAIL") << endl;
    } else {
      Serial << F("PASS") << endl;
    }
    N.showNetwork();
  }
}

// make sure we can correctly encode/decode message
void test_d(word low, word high) {
  Serial << F("==== TEST: encoding and decoding distances from:") << low << F(" to:") << high << endl;
  N.s = 0;
  
  for (byte i = 0; i < 10; i++) {
    word soln[N_NODES];
    for (byte n = 0; n < N_NODES; n++) {
      soln[n] = random(low, high);
      N.distance[n] = soln[n];
    }
    Serial << F("SOLN: ");
    N.showNetwork();

    // encode, decode
    N.encodeMessage();
    N.decodeMessage();
    
    if ( ! compare3(soln, N.distance) ) {
      Serial << F("FAIL") << endl;
    } else {
      Serial << F("PASS") << endl;
    }
  }


}

// make sure we can correctly send state messages
void test_state() {
  Serial << F("==== TEST: sending states") << endl;
  
  N.state = M_CALIBRATE;
  if ( ! N.sendState() ) {
    Serial << F("FAIL:\t");
  } else {
    Serial << F("PASS:\t");
  }
  N.showNetwork();

  N.state = M_PROGRAM;
  if ( ! N.sendState(PROGRAMMER_NODE) ) {
    Serial << F("FAIL:\t");
  } else {
    Serial << F("PASS:\t");
  }
  N.showNetwork();

  N.state = M_NORMAL;
  if ( ! N.sendState() ) {
    Serial << F("FAIL:\t");
  } else {
    Serial << F("PASS:\t");
  }
  N.showNetwork();

  N.state = M_NORMAL;
  if ( ! N.sendState() ) {
    Serial << F("FAIL:\t");
  } else {
    Serial << F("PASS:\t");
  }
  N.showNetwork();


}


// make sure we can correctly send distance messages
void test_message() {
  Serial << F("==== TEST: sending message") << endl;
  
  N.s = 0;
  N.distance[0] = random(0, 1023);
  N.distance[1] = random(0, 1023);
  N.distance[2] = random(0, 1023);
  N.encodeMessage();
   
  if ( ! N.sendMessage(PROGRAMMER_NODE) ) {
    Serial << F("FAIL:\t");
  } else {
    Serial << F("PASS:\t");
  }
  N.showNetwork();

  if ( ! N.sendMessage(123) ) {
    Serial << F("FAIL:\t");
  } else {
    Serial << F("PASS:\t");
  }
  N.showNetwork();

  if ( ! N.sendMessage(BROADCAST) ) {
    Serial << F("FAIL:\t");
  } else {
    Serial << F("PASS:\t");
  }
  N.showNetwork();

}



// test for packet loss
void test_packets() {
  Serial << F("==== TEST: sending message") << endl;

  int succ = 0;
  int Nsend = 1000;
  for(int i = 0; i<Nsend; i++) {
    N.s = 0;
    N.distance[0] = i;
    N.distance[1] = 0;
    N.distance[2] = 0;
    N.encodeMessage();
   
    if ( N.sendMessage(PROGRAMMER_NODE) ) succ++;
    else {  N.showNetwork(); }
  }

  Serial << F("Sent packets=") << Nsend << F("\tACKd=") << succ << F("\tpercent=") << ((unsigned long)succ*100UL)/Nsend << endl;
}


#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <FiniteStateMachine.h>

// Shadows specific libraries.
#include <Network.h>
#include <Distance.h>

// track our response using a finite state machine
/* 
 *  Startup <- Program 
 *  \  \
 *   \  \_ Node 10 _  (nodeID==10 has the special responsibility to bootstrap the Round-Robin)
 *    \             \
 *     -> Ready -> Range -> Send -> Sent -\ 
 *         ^                 ^______|    |
 *         |_____________________________|
 * 
 */

void program(); State Program = State(program); // programmer wants the airwaves
void startup(); State Startup = State(startup); // start here
void ready(); State Ready = State(ready); // ready to range when it's our turn
void range(); State Range = State(range); // run the range finder
void send(); State Send = State(send); // send the range information
void sent(); State Sent = State(sent); // after sending, make sure we hear downstream activity

FSM S = FSM(Startup); // start at Startup

// this should be at least the ranging time (~10ms), sending time (~5ms) and a fudge (~2ms)
// could we just be spamming too fast?
unsigned long resendInterval = 30UL; // ms
Metro resendTimer = Metro(resendInterval);

// this tells us who we receive the potato from, and who we give the potato to
byte recvFromNodeID, transToNodeID;

// track the amount of time things take
unsigned long myTime = 15000;
unsigned long notMyTime = 2*myTime;

// while the programmer is active, just wait
void program() {
  if ( N.state != M_PROGRAM ) S.transitionTo( Startup );

  Metro ledInterval = Metro(500);
  if( ledInterval.check() ) {
    static boolean ledState = false;
    digitalWrite(LED, ledState);
    ledState = ! ledState;
  }
}

// startup activities
void startup() {

  // track order for round-robin
  recvFromNodeID = N.left(N.myNodeID);
  transToNodeID = N.right(N.myNodeID);
  Serial << F("Startup.  myNodeID=") << N.myNodeID << F("\trecvFrom=") << recvFromNodeID << F("\t transTo=") << transToNodeID << endl;

  // do I need to get things moving?
  boolean bootstrap = false;
  if ( N.myNodeID == 10 ) bootstrap = true; // any node is a candidate for bootstrap responsibilities.  just pick one.
  Serial << F("Bootstrap responsibility? ") << bootstrap << endl;

  if ( bootstrap ) S.transitionTo( Range );
  else S.transitionTo( Ready );
}

// ready to range when it's our turn
void ready() {
  // figure out if it's our turn
  if ( N.senderNodeID == recvFromNodeID ) S.transitionTo( Range );
}

// range find
void range() {
  // visually flag that we've got the potato
  digitalWrite(LED, HIGH);
  // and score the amount of time since I gave up the potato
  notMyTime = (9*notMyTime + elapsedTime())/10; // running average

  // could the radio be getting in our way?
//  N.radio.sleep();
  
  // update distance
  word dist = D.read();
//  Serial << F("distance 0=") << dist << endl;
//  // could we just need more time to get a good reading?
//  dist = D.read();
//  Serial << F("distance 1=") << dist << endl;
//  dist = D.read();
//  Serial << F("distance 2=") << dist << endl;
  
  // updating the other's distance information so I can relay it with mine.
  N.decodeMessage(); 
  
  // record my distance
  N.distance[N.myIndex] = dist;

  Serial << F("set distance= ") << N.distance[N.myIndex] << endl;
  
  // set s so we can tell if all of the nodes are at the same codebase
  N.s = 2;

  // encode into the message
  N.encodeMessage();

  // ready to send.
  S.transitionTo( Send );
}

// send message
void send() {
  // bail out to programming mode immediately
  if ( N.state == M_PROGRAM ) {
    S.transitionTo( Program );
    return;
  }

  // send
  N.sendMessage(transToNodeID);

  // show
  Serial << F("SEND: "); N.showNetwork();

  // reset the resend timer
  resendTimer.reset();

  S.transitionTo( Sent );
}

// if we hear something fom downstream nodes, go to Ready.
// if we don't hear something from downstream nodes after an interval, go to Send (resend!)
void sent() {
  if ( N.senderNodeID == transToNodeID ) {
    // visually flag that we don't have the potato
    digitalWrite(LED, LOW);
    // and score the amount of time I had the potato
    myTime = (9*myTime + elapsedTime())/10; // running avergae

    // go to ready
    S.transitionTo( Ready );
  } else if ( resendTimer.check() ) {
    Serial << F("**RESENDING**\t");
    Serial << F("myTime (us)=") << myTime << F(" vs. resend time(us)=") << resendInterval*1000UL << endl;
    // resend
    S.transitionTo( Send );
  }
}

unsigned long elapsedTime() {
  static unsigned long then = micros();
  unsigned long now = micros();
  unsigned long delta = now - then;

  Serial << F("Timer elapsed (us)=") << delta << endl;
  then = now;
  return ( delta );
}

void setup() {
  Serial.begin(115200);

  // start the radio
  N.begin();

  // start the range finder
  D.begin();

  // wait enough time to get a reprogram signal.

  // while we're at it, calibrate and don't clobber each other doing so.
  unsigned long totalStartupDelay = 1000UL + 3UL*1000UL;
  unsigned long startupDelay = 1000UL + (unsigned long)(N.myIndex)*1000UL; 
  Serial << F("Startup. delay before calibration (ms)=") << startupDelay << endl;
  Metro startupTimer(startupDelay);
  startupTimer.reset();
  while (! startupTimer.check()) N.update();

  digitalWrite(LED, HIGH);
  D.calibrate();
  digitalWrite(LED, LOW);
  
  // delay for the rest of the time to let the other transceivers power up and calibrate
  startupDelay = totalStartupDelay-startupDelay;
  Serial << F("Startup. delay after calibration (ms)=") << startupDelay << endl;
  startupTimer.interval(startupDelay);
  startupTimer.reset();
  while (! startupTimer.check()) N.update();

}

void loop() {
  // update the radio traffic
  boolean haveTraffic = N.update();

  // bail out to programming mode immediately
  if ( N.state == M_PROGRAM ) S.transitionTo( Program );

  // configure to calibrate once
  if ( N.state == M_CALIBRATE ) {
    // MGD: TO-DO
    // D.begin();
    N.state = M_NORMAL;
  }

  if ( haveTraffic ) {
    Serial << F("RECV: "); N.showNetwork();
  }

  // update the FSM
  S.update();

}


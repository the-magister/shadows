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
State Program = State(program); // programmer wants the airwaves
State Startup = State(startup); // start here
State Ready = State(ready); // ready to range when it's our turn
State Range = State(range); // run the range finder
State Send = State(send); // send the range information
State Sent = State(sent); // after sending, make sure we hear downstream activity

FSM S = FSM(Startup); // start at Startup

// this should be at least the ranging time (~10ms) and sending time (~5ms)
#define RESEND_INTERVAL 15UL
Metro resendTimer = Metro(RESEND_INTERVAL);

// this tells us who we receive the potato from, and who we give the potato to
byte recvFromNodeID, transToNodeID;

// track the amount of time things take
unsigned long myTime = 15000;
unsigned long notMyTime = 2*myTime;

// when the programmer is active, just wait
void program() {
  if ( N.state != M_PROGRAM ) S.transitionTo(Startup);
}

// startup activities
void startup() {

  // track order for round-robin
  recvFromNodeID = N.left(N.myNodeID);
  transToNodeID = N.right(N.myNodeID);
  Serial << F("Startup.  myNodeID=") << N.myNodeID << F("\trecvFrom=") << recvFromNodeID << F("\t transTo=") << transToNodeID << endl;

  // setup for calibration
  D.calibrated = false;

  // do I need to get things moving?
  boolean bootstrap = false;
  if ( N.myNodeID == 10 ) bootstrap = true; // any node is a candidate for bootstrap responsibilities.  just pick one.
  Serial << F("Bootstrap responsibility? ") << bootstrap << endl;

  if ( bootstrap ) S.transitionTo( Ready );
  else S.transitionTo( Range );
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
  
  // might need to calibrate
  if ( ! D.calibrated ) D.calibrate();

  // update distance
  word dist = D.read();

  // updating the other's distance information so I can relay it with mine.
  N.decodeMessage(); 
  Serial << F("Decode Distances:\t") << N.distance[0] << F("\t") << N.distance[1] << F("\t") << N.distance[2] << endl;
  
  // record my distance
  N.distance[N.myIndex] = dist;

  // encode into the message
  Serial << F("Update Distances:\t") << N.distance[0] << F("\t") << N.distance[1] << F("\t") << N.distance[2] << endl;
  N.encodeMessage();
  Serial << F("Encode Distances:\t") << N.distance[0] << F("\t") << N.distance[1] << F("\t") << N.distance[2] << endl;

  // send, now.
  S.immediateTransitionTo( Send );
}

// send message
void send() {

  // send
  N.sendMessage(transToNodeID);

  // show
  Serial << F("SEND: "); N.showNetwork();

  // reset the resend timer
  resendTimer.reset();

  S.immediateTransitionTo( Sent );
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
    Serial << F("**RESENDING**") << endl;
    Serial << F("myTime+notMyTime (us)=") << myTime+notMyTime << F(" vs. resend time(us)=") << RESEND_INTERVAL*1000UL << endl;
    // resend
    S.immediateTransitionTo( Send );
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

  // wait enough time to get a reprogram signal
  Metro startupDelay(1000UL);
  while (! startupDelay.check()) N.update();

  // start the range finder
  D.begin();

}

void loop() {
  // update the radio traffic
  boolean haveTraffic = N.update();

  // bail out to programming mode immediately
  if ( N.state == M_PROGRAM ) S.immediateTransitionTo( Program );

  // configure to calibrate once
  if ( N.state = M_CALIBRATE ) {
    D.calibrated = false;
    N.state = M_NORMAL;
  }

  if ( haveTraffic ) {
    Serial << F("RECV: "); N.showNetwork();
  }

  // update the FSM
  S.update();

}


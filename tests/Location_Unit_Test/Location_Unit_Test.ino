#include <Streaming.h>
#include <Metro.h>

#include <Location.h>

word distance[N_NODES]={1,2,3};

void setup() {

  Serial.begin(115200);

  L.begin(distance);
 
  test_center();

  test_side();

  test_corner();

  Serial << F("====\nTests complete") << endl;
}

void loop() {
}

unsigned long elapsedTime() {
  static unsigned long then = micros();
  unsigned long now = micros();
  unsigned long delta = now - then;
  then = now;
  return ( delta );
}

boolean compare(word soln[N_NODES], word calc[N_NODES]) {
  Serial << F("soln\t") << F("\t") << soln[0]  << F("\t") << soln[1]  << F("\t") << soln[2] << endl;
  Serial << F("calc\t") << F("\t") << calc[0]  << F("\t") << calc[1]  << F("\t") << calc[2] << endl;
 
  if ( memcmp(soln, calc, sizeof(calc)) != 0 ) {
    return( false );
  } else {
    return( true );
  }
}

void test_center() {
  Serial << F("==== TEST: center") << endl;

  // softball test.  object is nearly in the center
  distance[0] = 393;
  distance[1] = 471;
  distance[2] = 447;

  unsigned long tic = elapsedTime();
  L.calculateLocation();
  tic = elapsedTime();
  Serial << F("calculateLocation(us)=") << tic << endl;
  
  Serial << endl << F("Ah:") << endl;
  word Ahsoln[N_NODES] = {262, 183, 209};
  Serial << F("result: ") << compare(Ahsoln, L.Ah) << endl;

  Serial << endl << F("Ab:") << endl;
  word Absoln[N_NODES] = {391,408,333};
  Serial << F("result: ") << compare(Absoln, L.Ab) << endl;

  Serial << endl << F("Cb:") << endl;
  word Cbsoln[N_NODES] = {402, 419, 310};
  Serial << F("result: ") << compare(Cbsoln, L.Cb) << endl;

  Serial << endl << F("Ch:") << endl;
  word Chsoln[N_NODES] = {262, 183, 210};
  Serial << F("result: ") << compare(Chsoln, L.Ch) << endl;

  Serial << endl << F("mArea:") << endl;
  word Areasoln[N_NODES] = {22741, 15884, 18141};
  Serial << F("result: ") << compare(Areasoln, L.Area) << endl;

}


void test_side() {
  Serial << F("==== TEST: side") << endl;

  // softball test.  object is nearly in the center
  distance[0] = 645;
  distance[1] = 511;
  distance[2] = 246;

  unsigned long tic = elapsedTime();
  L.calculateLocation();
  tic = elapsedTime();
  Serial << F("calculateLocation(us)=") << tic << endl;
  
  Serial << endl << F("Ah:") << endl;
  word Ahsoln[N_NODES] = {25, 200, 429};
  Serial << F("result: ") << compare(Ahsoln, L.Ah) << endl;

  Serial << endl << F("Ab:") << endl;
  word Absoln[N_NODES] = {510,143,482};
  Serial << F("result: ") << compare(Absoln, L.Ab) << endl;

  Serial << endl << F("Cb:") << endl;
  word Cbsoln[N_NODES] = {514, 41, 671};
  Serial << F("result: ") << compare(Cbsoln, L.Cb) << endl;

  Serial << endl << F("Ch:") << endl;
  word Chsoln[N_NODES] = {25, 225, 469};
  Serial << F("result: ") << compare(Chsoln, L.Ch) << endl;

  Serial << endl << F("mArea:") << endl;
  word Areasoln[N_NODES] = {2170, 17360, 37237};
  Serial << F("result: ") << compare(Areasoln, L.Area) << endl;

}

void test_corner() {
  Serial << F("==== TEST: corner") << endl;

  // hard test.  object is nearly in the corner
  distance[0] = 654;
  distance[1] = 101;
  distance[2] = 654;
  
  unsigned long tic = elapsedTime();
  L.calculateLocation();
  tic = elapsedTime();
  Serial << F("calculateLocation(us)=") << tic << endl;
  
  Serial << endl << F("Ah:") << endl;
  word Ahsoln[N_NODES] = {40, 573, 41};
  Serial << F("result: ") << compare(Ahsoln, L.Ah) << endl;

  Serial << endl << F("Ab:") << endl;
  word Absoln[N_NODES] = {93, 315, 653};
  Serial << F("result: ") << compare(Absoln, L.Ab) << endl;

  Serial << endl << F("Cb:") << endl;
  word Cbsoln[N_NODES] = {50, 372, 705};
  Serial << F("result: ") << compare(Cbsoln, L.Cb) << endl;

  Serial << endl << F("Ch:") << endl;
  word Chsoln[N_NODES] = {59, 576, 66};
  Serial << F("result: ") << compare(Chsoln, L.Ch) << endl;

  Serial << endl << F("Area:") << endl;
  word Areasoln[N_NODES] = {3472, 49737, 3558};
  Serial << F("result: ") << compare(Areasoln, L.Area) << endl;

}


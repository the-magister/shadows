#ifndef Location_h
#define Location_h

#include <Arduino.h>

#include <Streaming.h>

class Location {
  public:
    // initialize location
    void begin(byte left, byte across, byte right, long length);
    // set distance information
    void setDistance(byte node, long distance);
	
	// is there an object in the plane?
	boolean isObject(long maxDistance=72);
	
	// where is that object relative to my midpoint?
	long xPosition(); // in inches
	long yPosition(); // in inches
	
  private:
	byte leftNode, acrossNode, rightNode;
	
	float length;
    
	long leftD, acrossD, rightD;
	long x, y;

	boolean distanceCalculated;
	
	float areaFromDistances(float lA, float lB, float lC);
	float yFromArea(float area, float base);
	float xFromHeight(float y, float l);
	void triangulate();
};

extern Location L;

#endif

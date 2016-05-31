# "Shadows"

## Introduction

Briefly, the lighting will be addressable APA102s defining the perimeter of a equilateral triangle with 6' edges.  The lighting at each edge of the triangle is controlled by an ATMega328 uC (i.e. Moteuino) The location of an object penetrating the plane of the triangle will be known to each edge uC.  That is, when a human sticks their arm inside the plane and waves it around, that location information is updated real-time.

## Code Layout

* datasheets/ contains the relevant hardware data sheets for complex devices.
* gateway/WireLessProgramming/  contains the Windows/MacOS wireless programming tool, which allows you to program the light and sensor nodes over-the-air (OTA).
* libraries/ contains the relevant Arduino IDE libraries, and includes a custom, shared library (Network) shared by both light and sensor nodes.  
* tests/ unit tests.
* src/ primary location for gateway, lighting and sensor code.
	* Gateway_Programmer/ code base for device that serves as a gateway for WirelessProgramming tool.
	* Node_New/ bootstrap code to get a new Moteuino onto the wireless network for OTA programming.
	* Node_Location/ code base for device attached to ultrasound range finders.
	* Node_Lights/ code base for device attached to LED lighting.

## Physical Layout

             20             20, 21, 22: Node_Lights
       12 -------- 11     
         \        /         10, 11, 12: Node_Location
          \      / 
        21 \    / 22         Tens-digit: =2 Light, =1 Location
            \  /     _o      Ones-digit: matching digits indicate opposed nodes
             10       |\
      ______________ / > ___ 


Each of the vertices (10, 11, 12: Node_Location) have ultrasound range-finders.  These nodes range in a Round-Robin sequence (to prevent interference by each other).  After range finding, each node computes the location of an object in the plane of the triangle, using its latest range information and the previous updates from the other vertices.

To reduce the burden on the edges (20, 21, 22: Node_Lights), the sensors pre-calculate postional information relative to each edge.  For example, Node_Lights 22 is supplied the location of the object relative to itself.

Each of the edges (20, 21, 22: Node_Lights) translate the broadcast positional information to a animation of LEDs on the edges.  

## Installation and Getting Started

1. Install github tools.
2. Download source.
3. Install Arduino IDE.
    1. Open Arduino IDE
    2. Setting File->Preferences->Sketchbook Location to the location of installed source.  
	3. Setting Show Verbose Compilations.
	4. Restart Arudino IDE.
4. Compile changes to relevant code (Node_Lights, Node_Location).  Note .hex location.
5. Connect Gateway_Programmer/ Moteuino.  Note COM port.
6. Open gateway/WirelessProgramming tool.  Set COM port and .hex location from above.
	1. Set target nodeID (20, 21, 22: Node_Lights; 10, 11, 12: Node_Location), upload.  Repeat to the other two nodes.

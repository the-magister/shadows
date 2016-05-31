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
        21 \    / 22
            \  /     _o
             10       |\
      ______________ / > ___ 

Tens digit:
* 1 = nodes with ultrasound rangefinders (Node_Location)
* 2 = nodes with RGB LED strips (Node_Light)

## Installation and Getting Started

1. Install github tools.
2. Download source.
3. Install Arduino IDE.
    1. Open Arduino IDE
    2. Setting File->Preferences->Sketchbook Location to the location of installed source.  
	3. Setting Show Verbose Compilations.
	4. Restart Arudino IDE.
4. Compile changes to relevant code (Node_Lights, Node_Location).  Note .hex location.
5. Connect Gateway_Programmer/ Moteuino.  Node COM port.
6. Open gateway/WirelessProgramming tool.  Set COM port and .hex location from above.
7. Set target nodeID (20, 21, 22: Node_Lights; 10, 11, 12: Node_Location), upload.  Repeat to the other two nodes.
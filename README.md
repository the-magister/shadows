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

             20
       12 -------- 11
         \        /
          \      / 
        21 \    / 22
            \  /
             10

Tens digit:
    1 = nodes with ultrasound rangefinders (Node_Location)
    2 = nodes with RGB LED strips (Node_Light)

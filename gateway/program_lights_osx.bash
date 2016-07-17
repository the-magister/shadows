###
# program_lights_osx.bash
# 
# Program the shadows light array from an OSX terminal
# 
# Usage:
#    ./program_lights_osx.bash
#
# Requires:
#    pyserial (`pip install pyserial`)
# 
# Questions: Danne Stayskal <danne@stayskal.com>
###

### Find the HEX file
export HEX_FILE=`ls /var/folders/r*/*/T/build*/Node_Lights.ino.hex`
if [ -e "$HEX_FILE" ]
then
  echo "Found HEX file at $HEX_FILE"
else
	echo "Cannot find HEX file. Exiting."
	exit 1
fi

### Find the COM port
export COM_PORT=`ls /dev/cu.usbserial*`
if [ -e "$COM_PORT" ]
then
	echo "Found COM port at $COM_PORT"
else
	echo "Cannot find COM port. Exiting."
	exit 1
fi

### Clean up after ourselves
if [ -f Node_Lights.hex ]
then
  rm Node_Lights.hex
fi
if [ -f program_lights_output.log ]
then
	rm program_lights_output.log
fi
if [ -f program_lights_error.log ]
then
	rm program_lights_error.log
fi

### Push the build
cp $HEX_FILE Node_Lights.hex
for BOARD_NUMBER in 20 21 22
do
  echo "Pushing build to Moteuino $BOARD_NUMBER"
  python WirelessProgramming.py -f Node_Lights.hex -s $COM_PORT -t $BOARD_NUMBER 2> program_lights_error.log > program_lights_output.log
done

if [ `grep -e . -c program_lights_error.log` -gt 0 ]
then
	echo "Build errors:"
	tail -10 program_lights_error.log
	echo ""
	echo "Full error log in program_lights_error.log"
else
	echo "Build pushed with no errors"
fi

echo "Full programmer output is contained in program_lights_output.log"

HEX_FILE=`ls /tmp/build*/Node_Lights.ino.hex`
echo "found $HEX_FILE"
rm Node_Lights.hex
cp $HEX_FILE Node_Lights.hex

python WirelessProgramming.py -f Node_Lights.hex -s /dev/ttyUSB0 -t 20
python WirelessProgramming.py -f Node_Lights.hex -s /dev/ttyUSB0 -t 21
python WirelessProgramming.py -f Node_Lights.hex -s /dev/ttyUSB0 -t 22

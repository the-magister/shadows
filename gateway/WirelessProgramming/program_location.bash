HEX_FILE=`ls /tmp/build*/Node_Location.*.hex`
echo "found $HEX_FILE"
cp $HEX_FILE Node_Location.hex

python WirelessProgramming.py -f Node_Location.hex -s /dev/ttyUSB0 -t 10
python WirelessProgramming.py -f Node_Location.hex -s /dev/ttyUSB0 -t 11
python WirelessProgramming.py -f Node_Location.hex -s /dev/ttyUSB0 -t 12
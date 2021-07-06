#!/bin/sh

CFG="$1"
FWPATH="$2"

read -p " => Connect programmer to device and press ENTER to start flashing $FWPATH" dummy
echo "    *** Flashing firmware $FWPATH..."
openocd -f $CFG -c "program $FWPATH verify reset exit" || exit -1

#!/bin/bash

CFG="$1"
IMGPATH="$2"
ADDRESS="$3"

IMGPATH=${IMGPATH:=stm32f10x.bin}
ADDRESS=${ADDRESS:=0x08000000}

read -p " => Connect programmer to device and press ENTER to start uploading firmare $IMGPATH to $ADDRESS" dummy
echo "Uploading to $ADDRESS from $IMGPATH"
openocd -f $CFG -c "init; reset init; flash write_image erase $IMGPATH $ADDRESS; exit" || exit -1

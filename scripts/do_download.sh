#!/bin/bash

CFG="$1"
SIZEKB="$2"
IMGPATH="$3"
ADDRESS="$4"

SIZEKB = ${SIZEKB:=128}
SIZE=`expr $SIZEKB \* 1024`
IMGPATH = ${IMGPATH:=stm32f10x_downloaded.bin}
ADDRESS = ${ADDRESS:=0x08000000}

read -p " => Connect programmer to device and press ENTER to start downloading firmware from $ADDRESS to $IMGPATH" dummy
echo "Downloading $SIZE byte(s) starting from $ADDRESS to $IMGPATH"
openocd -f $CFG -c "init; reset init; dump_image $IMGPATH $ADDRESS $SIZE; exit" || exit -1

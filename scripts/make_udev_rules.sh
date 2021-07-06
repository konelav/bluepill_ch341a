#!/bin/sh

echo '
SUBSYSTEMS=="usb" ATTRS{idProduct}=="0101", ATTRS{idVendor}=="1366", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="0102", ATTRS{idVendor}=="1366", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="0103", ATTRS{idVendor}=="1366", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="0104", ATTRS{idVendor}=="1366", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="0105", ATTRS{idVendor}=="1366", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="0107", ATTRS{idVendor}=="1366", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="0108", ATTRS{idVendor}=="1366", MODE="0666"
' > /etc/udev/rules.d/99-jlink.rules

echo '
SUBSYSTEMS=="usb" ATTRS{idProduct}=="3744", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="3748", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="374a", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="374b", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="3752", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="3753", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="374d", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="374e", ATTRS{idVendor}=="0483", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="374f", ATTRS{idVendor}=="1366", MODE="0666"
' > /etc/udev/rules.d/99-stlink.rules

echo '
SUBSYSTEMS=="usb" ATTRS{idProduct}=="5512", ATTRS{idVendor}=="1a86", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="5523", ATTRS{idVendor}=="1a86", MODE="0666"
SUBSYSTEMS=="usb" ATTRS{idProduct}=="7523", ATTRS{idVendor}=="1a86", MODE="0666"
' > /etc/udev/rules.d/99-ch341a.rules

systemctl restart udev

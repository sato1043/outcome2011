#! /bin/sh

mkdir /dev/gpio

MAJOR=242
mknod /dev/gpio/ddsp0 c $MAJOR 0
mknod /dev/gpio/ddsp1 c $MAJOR 1

MAJOR=249
mknod /dev/gpio/swc0 c $MAJOR 0
mknod /dev/gpio/swc1 c $MAJOR 1
mknod /dev/gpio/swc2 c $MAJOR 2
mknod /dev/gpio/swc3 c $MAJOR 3
mknod /dev/gpio/swc4 c $MAJOR 4

MAJOR=243
mknod /dev/gpio/lcd0 c $MAJOR 0


#ls -l /dev/gpio

rmmod lcd
rmmod swc
rmmod ddsp

insmod ddsp.ko
insmod lcd.ko
insmod swc.ko

cat /proc/devices | grep ddsp
cat /proc/devices | grep lcd
cat /proc/devices | grep swc


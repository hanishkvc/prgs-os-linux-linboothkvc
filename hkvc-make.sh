#!/bin/bash

#KERPATH=/home/hanishkvc/hkvc/work/mysystem/nooktablet/ROMS/BN/source/1.4/distro/kernel/android-2.6.35
#CGCC=arm-linux-gnueabi- 
KERPATH=/hkvcwork/externel/AndroidDevices/BN/distro/kernel/android-2.6.35
CGCC=arm-eabi-

echo "Using KERNEL_DIR: $KERPATH"
echo "Remember to do atleast on kernel source"
echo "cp arch/arm/configs/android_4430BN_defconfig .config"
echo "make CROSS_COMPILE=arm-eabi- ARCH=arm oldconfig"
echo " make CROSS_COMPILE=arm-eabi- ARCH=arm scripts"

if [[ $1 == "clean" ]]; then
make KERNEL_DIR=$KERPATH ARCH=arm CROSS_COMPILE=$CGCC clean
else
make KERNEL_DIR=$KERPATH ARCH=arm CROSS_COMPILE=$CGCC all
fi

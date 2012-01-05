#!/bin/bash

if [[ $1 == "clean" ]]; then
make KERNEL_DIR=/home/hanishkvc/hkvc/work/mysystem/nooktablet/ROMS/BN/source/1.4/distro/kernel/android-2.6.35 ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- clean
else
make KERNEL_DIR=/home/hanishkvc/hkvc/work/mysystem/nooktablet/ROMS/BN/source/1.4/distro/kernel/android-2.6.35 ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- all
fi

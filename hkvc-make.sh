#!/bin/bash

DEVICE=DEVICE_BEAGLEXM
#DEVICE=DEVICE_NOOKTAB

if [[ $DEVICE == "DEVICE_BEAGLEXM" ]]; then
KERPATH=/hanishkvc/external/Android/rowboat-gingerbread/kernel
CGCC=arm-linux-gnueabi-
KERN_SYMS=$KERPATH/System.map
fi

if [[ $DEVICE == "DEVICE_NOOKTAB" ]]; then
KERPATH=/home/hanishkvc/hkvc/work/mysystem/nooktablet/ROMS/BN/source/1.4/distro/kernel/android-2.6.35
CGCC=arm-linux-gnueabi- 
#KERPATH=/hkvcwork/externel/AndroidDevices/BN/distro/kernel/android-2.6.35
#CGCC=arm-eabi-
KERN_SYMS=.target_kallsyms
fi


echo "Using KERNEL_DIR: $KERPATH"
echo "Remember to do atleast on kernel source"
echo "cp arch/arm/configs/android_4430BN_defconfig .config"
echo "make CROSS_COMPILE=arm-eabi- ARCH=arm oldconfig"
echo "make CROSS_COMPILE=arm-eabi- ARCH=arm scripts"
read -p "Press any key ..."

if [[ $1 == "clean" ]]; then

make KERNEL_DIR=$KERPATH ARCH=arm CROSS_COMPILE=$CGCC clean

elif [[ $1 == "check" ]]; then

if [[ "$DEVICE" == "DEVICE_NOOKTAB" ]]; then
	adb shell cat /proc/kallsyms > $KERN_SYMS
fi

cat lbhkvc_k.c | grep "kh_" | grep "= 0x"
ls $KERN_SYMS
cat $KERN_SYMS | grep "setup_mm_for_reboot"
cat $KERN_SYMS | grep "cpu_v7_proc_fin"
cat $KERN_SYMS | grep "cpu_v7_reset"
cat $KERN_SYMS | grep "v7_coherent_kern_range"
cat $KERN_SYMS | grep "disable_nonboot_cpus"
cat $KERN_SYMS | grep "show_pte"

else

make KERNEL_DIR=$KERPATH ARCH=arm CROSS_COMPILE=$CGCC DEVICE=$DEVICE all

fi


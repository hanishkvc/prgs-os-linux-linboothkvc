#!/bin/bash
# Support functions for common usecases in this project
# HKVC, GPL, Jan2012
#

LOCATION=HOME
#LOCATION=OTHER1

if [[ "$1" != "" ]]; then
CMD=$1
else
CMD="module"
fi

if [[ "$2" == "" ]]; then
#DEVICE=DEVICE_BEAGLEXM
DEVICE=DEVICE_NOOKTAB
#DEVICE=DEVICE_PANDA
else
DEVICE=$2
fi


echo "DEVICE: " $DEVICE
echo "LOCATION: " $LOCATION
echo "CMD: " $CMD
read -p "Hope this is fine..."

if [[ $DEVICE == "DEVICE_BEAGLEXM" ]]; then
	if [[ $LOCATION == "HOME" ]]; then
	KERPATH=/hanishkvc/external/Android/rowboat-gingerbread/kernel
	#CGCC=arm-linux-gnueabi-
	else
	KERPATH=/hkvcwork/externel/rowboat/gingerbread-nondsp/kernel
	#CGCC=arm-eabi-
	fi
	CGCC=arm-eabi-
	KERN_SYMS=$KERPATH/System.map
fi

if [[ $DEVICE == "DEVICE_NOOKTAB" ]]; then
	if [[ $LOCATION == "HOME" ]]; then
	KERPATH=/home/hanishkvc/hkvc/work/mysystem/nooktablet/ROMS/BN/source/1.4/distro/kernel/android-2.6.35
	#CGCC=arm-linux-gnueabi-
	else
	KERPATH=/hkvcwork/externel/AndroidDevices/BN/distro/kernel/android-2.6.35
	#CGCC=arm-eabi-
	fi
	CGCC=arm-eabi-
	KERN_SYMS=.target_kallsyms
fi

if [[ $DEVICE == "DEVICE_PANDA" ]]; then
	KERPATH=/hanishkvc/external/PandaBoard/omapzoom.org/kernel
	CGCC=arm-eabi-
	KERN_SYMS=$KERPATH/System.map
fi


echo "Using KERNEL_DIR: $KERPATH"
echo "Remember to do atleast on kernel source"
echo "cp arch/arm/configs/android_4430BN_defconfig .config"
echo "make CROSS_COMPILE=arm-eabi- ARCH=arm oldconfig"
echo "make CROSS_COMPILE=arm-eabi- ARCH=arm scripts"
read -p "Press any key ..."

if [[ $CMD == "clean" ]]; then

make KERNEL_DIR=$KERPATH ARCH=arm CROSS_COMPILE=$CGCC clean

fi

if [[ $CMD == "check" ]]; then

if [[ "$DEVICE" == "DEVICE_NOOKTAB" ]]; then
	adb shell cat /proc/kallsyms > $KERN_SYMS
fi

cat lbhkvc_k.c | grep "kh_" | grep "= 0x"
ls $KERN_SYMS
cat $KERN_SYMS | grep "ioremap_page"
cat $KERN_SYMS | grep "setup_mm_for_reboot"
cat $KERN_SYMS | grep "cpu_v7_proc_fin"
cat $KERN_SYMS | grep "cpu_v7_reset"
cat $KERN_SYMS | grep "v7_coherent_kern_range"
cat $KERN_SYMS | grep "disable_nonboot_cpus"
cat $KERN_SYMS | grep "show_pte"

fi

if [[ $CMD == "asm" ]] || [[ $CMD == "quick" ]]; then
	rm nirvana1.S
	if [[ $DEVICE == "DEVICE_NOOKTAB" ]]; then
	ln -s OmapNirvana3.S nirvana1.S
	else
	ln -s OmapNirvana3.S nirvana1.S
	fi
	make DEVICE=$DEVICE asm
	read -p "Hope everythin is fine with asm ..."
fi

if [[ $CMD == "nchild" ]] || [[ $CMD == "quick" ]]; then
	if [[ $DEVICE == "DEVICE_NOOKTAB" ]]; then
	cp -v $KERPATH/../../x-loader.hkvc/x-loadk.bin misc/Binaries/HKVC-$DEVICE-x-loadk.bin
	else
	cp -v $KERPATH/../x-loader.hkvc/x-loadk.bin misc/Binaries/HKVC-$DEVICE-x-loadk.bin
	fi
	cp -v misc/Binaries/HKVC-$DEVICE-x-loadk.bin hkvc.$DEVICE.nchild.bin
	rm hkvc.nchild.bin
	ln -s hkvc.$DEVICE.nchild.bin hkvc.nchild.bin
	make DEVICE=$DEVICE nchild
	read -p "Hope everythin is fine with nchild ..."
fi

if [[ $CMD == "install" ]]; then
	DEVICE=$DEVICE ./install.sh
fi

if [[ $CMD == "module" ]] || [[ $CMD == "quick" ]]; then

make KERNEL_DIR=$KERPATH ARCH=arm CROSS_COMPILE=$CGCC DEVICE=$DEVICE all

fi

if [[ $CMD == "uuencode" ]] || [[ $CMD == "quick" ]]; then
uuencode lbhkvc_km_$DEVICE.ko lbhkvc_km_$DEVICE.ko > lbhkvc_km.ko.uu
mv -v lbhkvc_km.ko.uu /tmp/send.uu
echo "On target use busybox rx recv.uu "
echo "On PC send send.uu using minicom's xmodem protocol"
echo "followed by busybox uudecode recv.uu on target"
fi


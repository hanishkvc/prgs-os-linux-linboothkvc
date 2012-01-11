#!/bin/bash
#
# qemu emulator with sdcard based usage support logic
# v08Jan2012
# HKVC, GPL, Jan2012
#

if [[ $# < 2 ]]; then
	echo "USAGE: $0 sd_image_raw mode"
	echo "mode = run|mount|copy|umount|all"
	exit
fi

df
echo "Free loop device is "
losetup -f
read -p "Hope loop0 is free and your image $1 is not already mounted ..."

mode=$2

if [[ "$mode" == "mount" ]] || [[ "$mode" == "all" ]]; then

	losetup /dev/loop0 $1
	kpartx -a /dev/loop0
	ls -l /dev/mapper/*
	mount /dev/mapper/loop0p1 /mnt/d1
	df
	read -p "The 1st partition should from the image $1 should be mounted ..."
fi


if [[ "$mode" == "copy" ]] || [[ "$mode" == "all" ]]; then
	#cp /hanishkvc/external/Android/rowboat-gingerbread/x-loader/MLO /mnt/d1/
	cp /hanishkvc/external/Android/rowboat-gingerbread/u-boot/u-boot.bin /mnt/d1/
	cp /hanishkvc/external/Android/rowboat-gingerbread/kernel/arch/arm/boot/uImage /mnt/d1/
	pushd /hanishkvc/external/Android/rowboat-gingerbread/out/target/product/beagleboard/root
	echo "Update initrd system if required, else exit"
	bash
	find . | cpio -o -H newc | gzip > /tmp/initrd
	popd
	mkimage -A arm -O linux -T ramdisk -a 0x81600000 -d /tmp/initrd /tmp/uInitrd 
	cp /tmp/uInitrd /mnt/d1/uInitrd
	ls -l /mnt/d1/
	read -p "Hope you have copied all required things to $1 ..."
fi

if [[ "$mode" == "umount" ]] || [[ "$mode" == "all" ]]; then
	umount /mnt/d1
	kpartx -d /dev/loop0
	losetup -d /dev/loop0
fi


if [[ "$mode" == "run" ]] || [[ "$mode" == "all" ]]; then
	qemu-system-arm -M beaglexm -clock unix -sd $1
fi


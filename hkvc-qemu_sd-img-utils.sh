#!/bin/bash
#
# qemu emulator with sdcard based usage support logic
# v08Jan2012
# HKVC, GPL, Jan2012
#

ANDPATH=/hkvcwork/externel/rowboat/gingerbread-nondsp
#ANDPATH=/hanishkvc/external/Android/rowboat-gingerbread


if [[ $# < 2 ]]; then
	echo "USAGE: $0 sd_image_raw mode"
	echo "mode = prepare|mount|copy|umount|run|all"
	echo "special mode = create"
	exit
fi

df
echo "Free loop device is "
sudo losetup -f
read -p "Hope loop0 is free and your image $1 is not already mounted ..."

mode=$2

if [[ "$mode" == "create" ]]; then
	dd if=/dev/zero of=$1 bs=1K count=400K
	sudo losetup /dev/loop0 $1
	echo "Remember to create 2 partitions VFat and Ext4"
	sudo fdisk /dev/loop0
	sudo fdisk -l /dev/loop0
	read -p "Are two partitions created"
	sudo kpartx -a /dev/loop0
	sudo mkfs.vfat -F 32 /dev/mapper/loop0p1
	sudo mkfs.ext4 /dev/mapper/loop0p2
fi

if [[ "$mode" == "prepare" ]] || [[ "$mode" == "all" ]]; then
	sudo losetup /dev/loop0 $1
	sudo kpartx -a /dev/loop0
	ls -l /dev/mapper/*
	read -p "Hope you can see the partitions listed above..."
fi

if [[ "$mode" == "mount" ]] || [[ "$mode" == "all" ]]; then
	sudo mount -o uid=hanishkvc /dev/mapper/loop0p1 /mnt/d1
	df
	read -p "The 1st partition should from the image $1 should be mounted ..."
fi


if [[ "$mode" == "copy" ]] || [[ "$mode" == "all" ]]; then
	cp $ANDPATH/x-loader/MLO /mnt/d1/
	cp $ANDPATH/u-boot/u-boot.bin /mnt/d1/
	cp $ANDPATH/kernel/arch/arm/boot/uImage /mnt/d1/
	pushd $ANDPATH/out/target/product/beagleboard/root
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
	sudo umount /mnt/d1
	sudo kpartx -d /dev/loop0
	sudo losetup -d /dev/loop0
fi


if [[ "$mode" == "run" ]] || [[ "$mode" == "all" ]]; then
	qemu-system-arm -M beaglexm -clock unix -sd $1
fi


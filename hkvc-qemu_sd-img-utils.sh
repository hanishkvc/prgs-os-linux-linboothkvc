#!/bin/bash
#
# qemu emulator with sdcard based usage support logic
# v08Jan2012
# HKVC, GPL, Jan2012
#


#ANDPATH=/hkvcwork/externel/rowboat/gingerbread-nondsp
ANDPATH=/hanishkvc/external/Android/rowboat-gingerbread


if [[ $# < 2 ]]; then
	echo "USAGE: $0 sd_image_raw mode"
	echo "mode = prepare|mount|boot.scr|copy|umount|all|run|runall"
	echo "special mode = create"
	exit
fi

df

if [[ "$2" == "runall" ]]; then
	mode="all"
	modeext="run"
else
	mode=$2
	modeext=""
fi

if [[ "$mode" != "run" ]]; then
	echo "Free loop device is "
	sudo losetup -f
	read -p "Hope loop0 is free and your image $1 is not already mounted ..."
fi

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

if [[ "$mode" == "boot.scr" ]] || [[ "$mode" == "all" ]]; then
	echo "Assumes that you are using u-boot and inturn it loads boot.scr from mmc by it having required env variables set to run it"
	echo mmc init 0 > /tmp/boot.txt
	echo fatload mmc 0:1 0x81000000 uImage >> /tmp/boot.txt
	echo fatload mmc 0:1 0x81800000 uInitrd >> /tmp/boot.txt
	echo setenv bootargs console=ttyO2 mpurate=auto >> /tmp/boot.txt
	echo echo bootargs used \${bootargs} >> /tmp/boot.txt
	echo bootm 0x81000000 0x81800000 >> /tmp/boot.txt
	mkimage -T script -A arm -d /tmp/boot.txt /tmp/boot.scr
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
	cp /tmp/boot.scr /mnt/d1/
	ls -l /mnt/d1/
	read -p "Hope you have copied all required things to $1 ..."
	if [[ $mode == "all" ]]; then
		pushd /mnt/d1
		echo "Update 1st partition if required, else exit"
		bash
		popd
	fi
fi

if [[ "$mode" == "umount" ]] || [[ "$mode" == "all" ]]; then
	sudo umount /mnt/d1
	sudo kpartx -d /dev/loop0
	sudo losetup -d /dev/loop0
fi

if [[ "$mode" == "run" ]] || [[ "$modeext" == "run" ]]; then
	#qemu-system-arm -M beaglexm -clock unix -sd $1 -nographic -serial pty -serial pty -serial pty -serial pty
	#qemu-system-arm -M beaglexm -sdl -clock unix -sd $1
	qemu-system-arm -M beaglexm -sdl -clock unix -sd $1 -s
fi


#
# Makefile for linux kernel module.
#
obj-m += lbhkvc_km_$(DEVICE).o
lbhkvc_km_$(DEVICE)-objs := lbhkvc_k.o gen_utils.o uart_utils.o
CFLAGS_lbhkvc_k.o += -D$(DEVICE)
CFLAGS_uart_utils.o += -D$(DEVICE)

#CROSS_COMPILE=arm-linux-gnueabi-
CROSS_COMPILE=arm-eabi-

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	#$(MAKE) V=1 -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm dummy1 hkvc.dummy1.bin || /bin/true

bloop1: bloop1.S
	rm dummy1.S
	ln -s bloop1.S dummy1.S

asm1: bloop1 asm

bloop2: bloop2.S
	rm dummy1.S
	ln -s bloop2.S dummy1.S

asm2: bloop2 asm

bloop3: bloop3.S
	rm dummy1.S
	ln -s bloop3.S dummy1.S

asm3: bloop3 asm

probe1: probe1.S
	rm dummy1.S
	ln -s probe1.S dummy1.S

asmp1: probe1 asm

asm:
	$(CROSS_COMPILE)gcc -nostdlib -march=armv7-a -o dummy1 dummy1.S -Ttext=0
	$(CROSS_COMPILE)objdump -d dummy1
	$(CROSS_COMPILE)objcopy -O binary -j .text dummy1 hkvc.dummy1.bin
	$(CROSS_COMPILE)objdump -D -m armv5te -b binary hkvc.dummy1.bin
	xxd -i hkvc.dummy1.bin > hkvc.dummy1.h

dump:
	$(CROSS_COMPILE)objdump -D -m armv5te -b binary hkvc.dummy1.bin | less
	$(CROSS_COMPILE)objdump -d lbhkvc_km.ko | less
	

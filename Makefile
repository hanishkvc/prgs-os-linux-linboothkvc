#
# Makefile for linux kernel module.
#
obj-m += lbhkvc_km.o
lbhkvc_km-objs := lbhkvc_k.o gen_utils.o uart_utils.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
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

asm:
	arm-linux-gnueabi-gcc -nostdlib -o dummy1 dummy1.S -Ttext=0
	arm-linux-gnueabi-objdump -d dummy1
	arm-linux-gnueabi-objcopy -O binary -j .text dummy1 hkvc.dummy1.bin
	arm-linux-gnueabi-objdump -D -m armv5te -b binary hkvc.dummy1.bin
	xxd -i hkvc.dummy1.bin > hkvc.dummy1.h

dump:
	arm-linux-gnueabi-objdump -D -m armv5te -b binary hkvc.dummy1.bin | less
	arm-linux-gnueabi-objdump -d lbhkvc_km.ko | less
	

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
	rm lbhkvc_km.ko || /bin/true
	ln -s lbhkvc_km_$(DEVICE).ko lbhkvc_km.ko
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm nirvana1 hkvc.nirvana1.bin || /bin/true
	rm lbhkvc_km.ko || /bin/true

nchild:
	xxd -i hkvc.nchild.bin > hkvc.nchild.h

asm:
	$(CROSS_COMPILE)gcc -D$(DEVICE) -nostdlib -march=armv7-a -o nirvana1 nirvana1.S -Ttext=0
	$(CROSS_COMPILE)objdump -d nirvana1
	$(CROSS_COMPILE)objcopy -O binary -j .text nirvana1 hkvc.nirvana1.bin
	$(CROSS_COMPILE)objdump -D -m armv7-a -b binary hkvc.nirvana1.bin || /bin/true
	$(CROSS_COMPILE)objdump -D -m armv5te -b binary hkvc.nirvana1.bin
	xxd -i hkvc.nirvana1.bin > hkvc.nirvana1.h

dump:
	$(CROSS_COMPILE)objdump -D -m armv5te -b binary hkvc.nirvana1.bin | less
	$(CROSS_COMPILE)objdump -d lbhkvc_km.ko | less

dumpko:
	$(CROSS_COMPILE)objdump -d -x lbhkvc_km.ko | less


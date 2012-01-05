#
# Makefile for linux kernel module.
#
obj-m += lbhkvc_k.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

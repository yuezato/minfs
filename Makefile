KERNEL_DIR = /lib/modules/$(shell uname -r)/build 
BUILD_DIR := $(shell pwd)
VERBOSE := 0

ccflags-y := -g -O0

obj-m := minfs.o

all:
	make -C $(KERNEL_DIR) M=$(BUILD_DIR) KBUILD_VERBOSE=$(VERBOSE) modules 
clean:
	rm -rf *.o *.ko *.mod.c *.symvers *.order .tmp_versions .minfs.*

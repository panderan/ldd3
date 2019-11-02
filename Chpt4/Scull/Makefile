KERNEL_DIR=../../kernel-source
CUR_DIR=$(shell pwd)
CROSS_COMPILE_PREFIX=x86_64-linux-uclibc-

EXTRA_CFLAGS+=-DSCULL_DEBUG

obj-m := scull.o
scull-objs := proc_seq.o proc.o fops.o main.o

all:
	make -C $(KERNEL_DIR) M=$(CUR_DIR) \
			CROSS_COMPILE=${CROSS_COMPILE_PREFIX} modules

modules_install:
	make -C $(KERNEL_DIR) M=$(CUR_DIR) \
			CROSS_COMPILE=${CROSS_COMPILE_PREFIX} \
			INSTALL_MOD_PATH=${INSTALL_MOD_PATH} modules_install
	install	./scull.init ${INSTALL_MOD_PATH}/etc/init.d

clean:
	make -C $(KERNEL_DIR) M=$(CUR_DIR) \
			CROSS_COMPILE=${CROSS_COMPILE_PREFIX} clean 


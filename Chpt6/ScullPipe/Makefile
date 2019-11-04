KERNEL_DIR=../../kernel-source
CUR_DIR=$(shell pwd)
CROSS_COMPILE_PREFIX=x86_64-linux-uclibc-

obj-m := scullpipe.o
scullpipe-objs := main.o sc_pipe.o sc_pipe_fops.o

all:
	make -C $(KERNEL_DIR) M=$(CUR_DIR) \
			CROSS_COMPILE=${CROSS_COMPILE_PREFIX} modules

modules_install:
	make -C $(KERNEL_DIR) M=$(CUR_DIR) \
			CROSS_COMPILE=${CROSS_COMPILE_PREFIX} \
			INSTALL_MOD_PATH=${INSTALL_MOD_PATH} modules_install
	install	./scullpipe.init ${INSTALL_MOD_PATH}/etc/init.d

clean:
	make -C $(KERNEL_DIR) M=$(CUR_DIR) \
			CROSS_COMPILE=${CROSS_COMPILE_PREFIX} clean 


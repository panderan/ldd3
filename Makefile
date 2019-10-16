KERNEL_DIR_NAME=kernel-source

all:
	make -C Hello

modules_install:
	make -C Hello INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

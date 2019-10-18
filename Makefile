KERNEL_DIR_NAME=kernel-source

all:
	make -C Hello
	make -C Scull 

modules_install:
	make -C Hello INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install
	make -C Scull INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	make -C Hello clean 
	make -C Scull clean 

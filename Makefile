KERNEL_DIR_NAME=kernel-source

all:
	make -C Chpt2-Hello
	make -C Chpt5-Scull 

modules_install:
	make -C Chpt2-Hello INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install
	make -C Chpt4-Scull INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	make -C Chpt2-Hello clean 
	make -C Chpt4-Scull clean 

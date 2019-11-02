KERNEL_DIR_NAME=kernel-source

all:
	make -C Chpt2-Hello
	make -C Chpt6-Scull 

modules_install:
	make -C Chpt2-Hello INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install
	make -C Chpt6-Scull INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	make -C Chpt2-Hello clean 
	make -C Chpt6-Scull clean 

/* 内核模块的基本头文件 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

/* 其他头文件 */
#include <linux/fs.h>

#include "scullpipe.h"
#include "sc_pipe_fops.h"


MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");




/// 模块清除函数
/**
  * 清除函数可以被用来处理初始化失败的场景，因此，它必须能够正确的处理一些还没
  * 被初始化的地方。
  */
void scullpipe_cleanup_module(void)
{
	int i = 0;
	dev_t devno = MKDEV(scullpipe_major, scullpipe_minor);

	/* 清理设备结构体 */
	if (scullpipe_devices) {
		for (i = 0; i < scullpipe_nr_devs; i++) {
			cdev_del(&scullpipe_devices[i].cdev);
		}
		kfree(scullpipe_devices);
	}

    /* 如果设备注册失败则不应调用 cleanup_module */
	unregister_chrdev_region(devno, scullpipe_nr_devs);
}


int scullpipe_init_module(void)
{
	int result = 0;

    if((result=scullpipe_create()) != 0) {
        goto fail;
    }
	return 0;

fail:
	scullpipe_cleanup_module();
	return result;
}

module_init(scullpipe_init_module);
module_exit(scullpipe_cleanup_module);


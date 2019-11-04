/* 内核模块的基本头文件 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include "scull.h"
#include "proc.h"


MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");


/// 申请设备号
int demand_device_num(char *devname, int *major, int *minor, int nr)
{
	int result = 0;
	dev_t dev = 0;

    // 获得一组次设备号，主设备号通过动态获得除非在模块载入时被指定
	if (*major) {
        // 指定分配
		dev = MKDEV(*major, *minor);
		result = register_chrdev_region(dev, nr, devname);
	} else {
        // 动态分配
		result = alloc_chrdev_region(&dev, *minor, nr, devname);
		*major = MAJOR(dev);
	}

	if (result < 0) {
		printk(KERN_WARNING "%s: can't get major %d\n", devname, *major);
	}
	return result;
}


/// 模块清除函数
/**
  * 清除函数可以被用来处理初始化失败的场景，因此，它必须能够正确的处理一些还没
  * 被初始化的地方。
  */
void scull_cleanup_module(void)
{

#ifdef SCULL_DEBUG
    scull_remove_proc();
#endif
}


/// 模块初始化函数
int scull_init_module(void)
{
	int result = 0;

    result = demand_device_num("scull", &scull_major, &scull_minor, scull_nr_devs);
    if (result != 0) {
        return result;
    }
    result = scull_create();
    if (result != 0) {
        goto fail;
    }

#ifdef SCULL_DEBUG
    scull_create_proc();
#endif

	return 0;

 fail:
	scull_cleanup_module();
	return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);


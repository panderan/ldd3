/* 内核模块的基本头文件 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

/* 其他头文件 */
#include <linux/fs.h>

#include "sleepy.h"
#include "fops.h"


int sleepy_major =   SLEEPY_MAJOR;
int sleepy_minor =   0;
int sleepy_nr_devs = SLEEPY_NR_DEVS;


/*
 * 在载入模块时可指定的参数.
 */
module_param(sleepy_major, int, S_IRUGO);
module_param(sleepy_minor, int, S_IRUGO);
module_param(sleepy_nr_devs, int, S_IRUGO);


MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");


/* sleepy 的设备结构，在sleepy_init_module 中分配内存 */
struct sleepy_dev *sleepy_devices;	


/* 实现相关的系统调用 */
struct file_operations sleepy_fops = {
	.owner =    THIS_MODULE,
	.open =     sleepy_open,
	.release =  sleepy_release,
	.read =     sleepy_read,
	.write =    sleepy_write,
	.llseek =   sleepy_llseek,
};


/// 模块清除函数
/**
  * 清除函数可以被用来处理初始化失败的场景，因此，它必须能够正确的处理一些还没
  * 被初始化的地方。
  */
void sleepy_cleanup_module(void)
{
	int i = 0;
	dev_t devno = MKDEV(sleepy_major, sleepy_minor);

	/* 清理设备结构体 */
	if (sleepy_devices) {
		for (i = 0; i < sleepy_nr_devs; i++) {
			cdev_del(&sleepy_devices[i].cdev);
		}
		kfree(sleepy_devices);
	}

    /* 如果设备注册失败则不应调用 cleanup_module */
	unregister_chrdev_region(devno, sleepy_nr_devs);
}

/// 初始化设备
/**
 * 设置每一个设备结构体，并进行安装
 */
static void sleepy_setup_cdev(struct sleepy_dev *dev, int index)
{
	int err = 0;
    int devno = MKDEV(sleepy_major, sleepy_minor + index);
    
    dev->flag = 0;
    init_waitqueue_head(&dev->my_queue);
	cdev_init(&dev->cdev, &sleepy_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &sleepy_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding sleepy%d", err, index);
}


int sleepy_init_module(void)
{
	int result = 0, i = 0;
	dev_t dev = 0;

/*
 * 获得一组次设备号，主设备号通过动态获得除非在模块载入时被指定
 */
	if (sleepy_major) {
        /* 指定分配 */
		dev = MKDEV(sleepy_major, sleepy_minor);
		result = register_chrdev_region(dev, sleepy_nr_devs, "sleepy");
	} else {
        /* 动态分配 */
		result = alloc_chrdev_region(&dev, sleepy_minor, sleepy_nr_devs, "sleepy");
		sleepy_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "sleepy: can't get major %d\n", sleepy_major);
		return result;
	}

    /* 
     * 申请设备结构地址，由于设备数可在模块载入时指定因此不能将该结构作为静态变量
	 */
	sleepy_devices = kmalloc(sleepy_nr_devs * sizeof(struct sleepy_dev), GFP_KERNEL);
	if (!sleepy_devices) {
		result = -ENOMEM;
		goto fail;
	}
	memset(sleepy_devices, 0, sleepy_nr_devs * sizeof(struct sleepy_dev));

    /* 初始化每一个设备 */
	for (i = 0; i < sleepy_nr_devs; i++) {
		sleepy_setup_cdev(&sleepy_devices[i], i);
	}

	return 0;

 fail:
	sleepy_cleanup_module();
	return result;
}

module_init(sleepy_init_module);
module_exit(sleepy_cleanup_module);


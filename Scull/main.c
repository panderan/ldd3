/* 内核模块的基本头文件 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

/* 其他头文件 */
#include <linux/fs.h>

#include "scull.h"
#include "fops.h"


int scull_major =   SCULL_MAJOR;
int scull_minor =   0;
int scull_nr_devs = SCULL_NR_DEVS;
int scull_quantum = SCULL_QUANTUM;
int scull_qset =    SCULL_QSET;


/*
 * 在载入模块时可指定的参数.
 */
module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);


MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");


/* scull 的设备结构，在scull_init_module 中分配内存 */
struct scull_dev *scull_devices;	


/* 实现相关的系统调用 */
struct file_operations scull_fops = {
	.owner =    THIS_MODULE,
	.llseek =   scull_llseek,
	.read =     scull_read,
	.write =    scull_write,
	.open =     scull_open,
	.release =  scull_release,
};


/// 模块清除函数
/**
  * 清除函数可以被用来处理初始化失败的场景，因此，它必须能够正确的处理一些还没
  * 被初始化的地方。
  */
void scull_cleanup_module(void)
{
	int i = 0;
	dev_t devno = MKDEV(scull_major, scull_minor);

	/* 清理设备结构体 */
	if (scull_devices) {
		for (i = 0; i < scull_nr_devs; i++) {
			scull_trim(scull_devices + i);
			cdev_del(&scull_devices[i].cdev);
		}
		kfree(scull_devices);
	}

    /* 如果设备注册失败则不应调用 cleanup_module */
	unregister_chrdev_region(devno, scull_nr_devs);
}

/// 初始化设备
/**
 * 设置每一个设备结构体，并进行安装
 */
static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err = 0;
    int devno = MKDEV(scull_major, scull_minor + index);
    
	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}


int scull_init_module(void)
{
	int result, i;
	dev_t dev = 0;

/*
 * 获得一组次设备号，主设备号通过动态获得除非在模块载入时被指定
 */
	if (scull_major) {
        /* 指定分配 */
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} else {
        /* 动态分配 */
		result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
				"scull");
		scull_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

    /* 
     * 申请设备结构地址，由于设备数可在模块载入时指定因此不能将该结构作为静态变量
	 */
	scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
	if (!scull_devices) {
		result = -ENOMEM;
		goto fail;
	}
	memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

    /* 初始化每一个设备 */
	for (i = 0; i < scull_nr_devs; i++) {
		scull_devices[i].quantum = scull_quantum;
		scull_devices[i].qset = scull_qset;
        init_MUTEX(&scull_devices[i].sem);
		scull_setup_cdev(&scull_devices[i], i);
	}

	return 0;

 fail:
	scull_cleanup_module();
	return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);


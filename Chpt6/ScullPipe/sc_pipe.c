#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "scullpipe.h"
#include "sc_pipe_fops.h"

int scullpipe_major =   SCULLPIPE_MAJOR;
int scullpipe_minor =   0;
int scullpipe_nr_devs = SCULLPIPE_NR_DEVS;

// 在载入模块时可指定的参数.
module_param(scullpipe_major, int, S_IRUGO);
module_param(scullpipe_minor, int, S_IRUGO);
module_param(scullpipe_nr_devs, int, S_IRUGO);

// scullpipe 的设备结构，在scullpipe_init_module 中分配内存
struct scullpipe_dev *scullpipe_devices;


/// 分配设备号
int scullpipe_require_devnum(void)
{
    int result = 0;
	dev_t dev = 0;

	if (scullpipe_major) {
        // 指定分配
		dev = MKDEV(scullpipe_major, scullpipe_minor);
		result = register_chrdev_region(dev, scullpipe_nr_devs, "scullpipe");
	} else {
        // 动态分配
		result = alloc_chrdev_region(&dev, scullpipe_minor, scullpipe_nr_devs, "scullpipe");
		scullpipe_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scullpipe: can't get major %d\n", scullpipe_major);
	}

    return result;
}

/// 初始化单个设备
/**
 * 设置每一个设备结构体，并进行安装
 */
void scullpipe_setup_cdev(struct scullpipe_dev *dev, int index)
{
	int err = 0;
    int devno = MKDEV(scullpipe_major, scullpipe_minor + index);
    
    init_waitqueue_head(&dev->inq);
    init_waitqueue_head(&dev->outq);
    init_MUTEX(&dev->sem);
    dev->buffer = NULL;
    dev->end = NULL;
    dev->rp = NULL;
    dev->wp = NULL;
    dev->buffersize = 0;
    dev->nreaders = 0;
    dev->nwriters = 0;

	cdev_init(&dev->cdev, &scullpipe_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scullpipe_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding scullpipe%d", err, index);
}

/// 创建 scullpipe 设备
int scullpipe_create(void)
{
    int result = 0, i = 0;

    // 申请设备号
    if ((result=scullpipe_require_devnum()) != 0) {
        return result;
    }

    // 申请设备结构地址，由于设备数可在模块载入时指定因此不能将该结构作为静态变量
	scullpipe_devices = kmalloc(scullpipe_nr_devs * sizeof(struct scullpipe_dev), GFP_KERNEL);
	if (!scullpipe_devices) {
        return -ENOMEM;
	}
	memset(scullpipe_devices, 0, scullpipe_nr_devs * sizeof(struct scullpipe_dev));

    // 初始化每一个设备
	for (i = 0; i < scullpipe_nr_devs; i++) {
		scullpipe_setup_cdev(&scullpipe_devices[i], i);
	}

    return 0;
}

/// 释放 scullpipe 设备
void scullpipe_destroy(void)
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

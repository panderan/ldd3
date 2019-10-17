/*
 * scull.h -- definitions for the char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: scull.h,v 1.15 2004/11/04 17:51:18 rubini Exp $
 */

#ifndef _SCULL_H_
#define _SCULL_H_

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0       /* 主设备号，0 - 动态分配 */
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4     /* 设备数量，scull0 到 scull3 */
#endif


/*
 * 实际的设备是一段可变长度的内存区域，使用链表进行连接
 * 
 * "scull_dev->data" 指向一个指针数组，其中的每一个指针指向一块 SCULL_QUANTUM 字节大小的内存区域
 *
 * quantum-set 数组的大小为 SCULL_QSET
 */

#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET    1000
#endif


/*
 * Representation of scull quantum sets.
 */
struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
	struct scull_qset *data;  /* Pointer to first quantum set */
	int quantum;              /* the current quantum size */
	int qset;                 /* the current array size */
	unsigned long size;       /* amount of data stored here */
	unsigned int access_key;  /* used by sculluid and scullpriv */
	struct semaphore sem;     /* mutual exclusion semaphore     */
	struct cdev cdev;	  /* Char device structure		*/
};

/*
 * The different configurable parameters
 */
extern int scull_major;     /* main.c */
extern int scull_nr_devs;
extern int scull_quantum;
extern int scull_qset;

extern int scull_p_buffer;	/* pipe.c */


/*
 * Prototypes for shared functions
 */

int     scull_p_init(dev_t dev);
void    scull_p_cleanup(void);

int     scull_trim(struct scull_dev *dev);
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
loff_t  scull_llseek(struct file *filp, loff_t off, int whence);
int     scull_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

#endif /* _SCULL_H_ */

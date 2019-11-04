#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/semaphore.h>
#include <linux/cdev.h>

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0       /* 主设备号，0 - 动态分配 */
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4     /* 设备数量，scull0 到 scull3 */
#endif


/*
 * 实际的设备是一段可变长度的内存区域，使用链表进行连接
 * "scull_dev->data" 指向一个指针数组，其中的每一个指针指向一块 SCULL_QUANTUM 字节大小的内存区域
 * quantum-set 数组的大小为 SCULL_QSET
 */

#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET    1000
#endif


/*
 * 量子集
 */
struct scull_qset {
	void **data;                /* 该量子集中的数据，指向一个 SCULL_QSET 大小的
                                   指针数组, 每一个指针又指向 SCULL_QUANTUM  大
                                   小的内存区域 */
	struct scull_qset *next;    /* 链表结构，指向下一个量子集 */
};

struct scull_dev {
	struct scull_qset *data;    /* scull_qset 链表的第一个节点 */
	int quantum;                /* 量子大小 */
	int qset;                   /* 数组大小（scull_qset.data 的长度） */
	unsigned long size;         /* 总数据量大小 */
    struct semaphore sem;       /* 互斥信号量 */
	struct cdev cdev;	        /* 字符设备结构	*/
};

/*
 *  可配置参数 
 */
extern int scull_major;     /* scull_dev.c */
extern int scull_minor;     /* scull_dev.c */
extern int scull_nr_devs;
extern int scull_quantum;
extern int scull_qset;

extern struct scull_dev *scull_devices;	

int scull_create(void);
void scull_destroy(void);

#endif /* _SCULL_H_ */

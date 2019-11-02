#ifndef _SCULL_FOPS_
#define _SCULL_FOPS_

#include "scull.h"

#define SCULL_IOC_MAGIC 'k'

#define SCULL_IOCRESET      _IO(SCULL_IOC_MAGIC, 0)
/* S 通过指针设置值 
 * T 通过值设置值
 * G 通过指针读取值
 * Q 通过值读取值
 * X 通过指针设置新值并返回旧值
 * H 通过值设置新值并返回旧值
 */
#define SCULL_IOC_S_QUANTUM     _IOW(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOC_S_QSET        _IOW(SCULL_IOC_MAGIC, 2, int)

#define SCULL_IOC_T_QUANTUM     _IO(SCULL_IOC_MAGIC, 3)
#define SCULL_IOC_T_QSET        _IO(SCULL_IOC_MAGIC, 4)

#define SCULL_IOC_G_QUANTUM     _IOR(SCULL_IOC_MAGIC, 5, int)
#define SCULL_IOC_G_QSET        _IOR(SCULL_IOC_MAGIC, 6, int)

#define SCULL_IOC_Q_QUANTUM     _IO(SCULL_IOC_MAGIC, 7)
#define SCULL_IOC_Q_QSET         _IO(SCULL_IOC_MAGIC, 8)

#define SCULL_IOC_X_QUANTUM     _IOWR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOC_X_QSET         _IOWR(SCULL_IOC_MAGIC, 10, int)

#define SCULL_IOC_H_QUANTUM     _IO(SCULL_IOC_MAGIC, 11)
#define SCULL_IOC_H_QSET         _IO(SCULL_IOC_MAGIC, 12)

#define SCULL_P_IOC_T_SIZE      _IO(SCULL_IOC_MAGIC, 13)
#define SCULL_P_IOC_Q_SIZE      _IO(SCULL_IOC_MAGIC, 14)

#define SCULL_IOC_MAXNR 14

int scull_trim(struct scull_dev *dev);
int scull_open(struct inode *inode, struct file *filp);
int scull_release(struct inode *inode, struct file *filp);
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
loff_t scull_llseek(struct file *filp, loff_t off, int whence);
int scull_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif

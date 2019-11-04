#ifndef _SLEEPY_FOPS_
#define _SLEEPY_FOPS_

#include "sleepy.h"

int sleepy_open(struct inode *inode, struct file *filp);
int sleepy_release(struct inode *inode, struct file *filp);
ssize_t sleepy_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t sleepy_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
loff_t sleepy_llseek(struct file *filp, loff_t off, int whence);

#endif

#ifndef _SCULLPIPE_FOPS_
#define _SCULLPIPE_FOPS_

#include "scullpipe.h"

#ifndef SCULLPIPE_BUFFER_SIZE
#define SCULLPIPE_BUFFER_SIZE 4000     /* 设备数量，SCULLPIPE0 到 SCULLPIPE1 */
#endif

int scullpipe_open(struct inode *inode, struct file *filp);
int scullpipe_release(struct inode *inode, struct file *filp);
ssize_t scullpipe_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t scullpipe_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

extern struct file_operations scullpipe_fops;

#endif

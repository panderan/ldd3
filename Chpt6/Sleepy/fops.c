#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include "fops.h"


int sleepy_open(struct inode *inode, struct file *filp)
{
    struct sleepy_dev *dev = NULL;

    dev = container_of(inode->i_cdev, struct sleepy_dev, cdev);
    filp->private_data = dev;
    return 0;
}

int sleepy_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t sleepy_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct sleepy_dev *dev = filp->private_data;

    printk(KERN_DEBUG "Process %i(%s) going to sleep\n", current->pid, current->comm);
    wait_event_interruptible(dev->my_queue, dev->flag != 0);

    dev->flag = 0;
    printk(KERN_DEBUG "awoken %i(%s)\n", current->pid, current->comm);
    return 0; /* EOF */
}

ssize_t sleepy_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct sleepy_dev *dev = filp->private_data;

    printk(KERN_DEBUG "Procecss %i(%s) awakening the readers ... \n",
                                current->pid, current->comm);
    dev->flag = 1;
    wake_up_interruptible(&dev->my_queue);
    return count; /* 成功并避免重试 */
    
}

loff_t sleepy_llseek(struct file *filp, loff_t off, int whence)
{
    loff_t newpos = 0;
    filp->f_pos = newpos;
    return newpos;
}


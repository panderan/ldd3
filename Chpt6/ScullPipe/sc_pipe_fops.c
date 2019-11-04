#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#include "sc_pipe_fops.h"


int scullpipe_buffer_size = SCULLPIPE_BUFFER_SIZE;

/* 实现相关的系统调用 */
struct file_operations scullpipe_fops = {
	.owner =    THIS_MODULE,
	.open =     scullpipe_open,
	.release =  scullpipe_release,
	.read =     scullpipe_read,
	.write =    scullpipe_write,
	.llseek =   no_llseek,

};


int scullpipe_open(struct inode *inode, struct file *filp)
{
    struct scullpipe_dev *dev = NULL;

    dev = container_of(inode->i_cdev, struct scullpipe_dev, cdev);
    filp->private_data = dev;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (dev->buffer == NULL) {
        dev->buffer = kmalloc(scullpipe_buffer_size, GFP_KERNEL);
        if (dev->buffer == NULL) {
            up(&dev->sem);
            return -ENOMEM;
        }
    }
    dev->buffersize = scullpipe_buffer_size;
    dev->end = dev->buffer + dev->buffersize;
    dev->rp = dev->buffer;
    dev->wp = dev->buffer;

    if (filp->f_mode & FMODE_READ)
        dev->nreaders++;
    if (filp->f_mode & FMODE_WRITE)
        dev->nwriters++;

    up(&dev->sem);
    return nonseekable_open(inode, filp);
}

int scullpipe_release(struct inode *inode, struct file *filp)
{
    struct scullpipe_dev *dev = filp->private_data;

    down(&dev->sem);
    if (filp->f_mode & FMODE_READ)
        dev->nreaders--;
    if (filp->f_mode & FMODE_WRITE)
        dev->nwriters--;
    if (dev->nreaders + dev->nwriters == 0) {
        kfree(dev->buffer);
        dev->buffer = NULL;
    }
    up(&dev->sem);
    return 0;
}

ssize_t scullpipe_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scullpipe_dev *dev = filp->private_data;
    
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    
    while (dev->rp == dev->wp) {
        up(&dev->sem);
        if (filp->f_flags & O_NONBLOCK) 
            return -EAGAIN;
        
        printk(KERN_DEBUG "%i(%s) reader is going to sleep\n", current->pid, current->comm);
        if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
            return -ERESTARTSYS;
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
    }

    if (dev->wp > dev->rp)
        count = min(count, (size_t)(dev->wp - dev->rp));
    else
        count = min(count, (size_t)(dev->end - dev->rp));

    if (copy_to_user(buf, dev->rp, count)) {
        up(&dev->sem);
        return -EFAULT;
    }
    dev->rp += count;
    if (dev->rp == dev->end)
        dev->rp = dev->buffer;

    up(&dev->sem);
    
    wake_up_interruptible(&dev->outq);
    printk(KERN_DEBUG "%i(%s) did read %li bytes\n", current->pid, current->comm, count);
    return count;
}

static int spacefree(struct scullpipe_dev *dev) 
{
    if (dev->rp == dev->wp)
        return dev->buffersize - 1;
    return (dev->buffersize - (dev->wp - dev->rp)) % dev->buffersize - 1;
}

static int scullpipe_getwritespace(struct scullpipe_dev *dev, struct file *filp)
{
    while (spacefree(dev) == 0) {
        /* FULL */
        DEFINE_WAIT(wait);
        up(&dev->sem);
        if (filp->f_flags & O_NONBLOCK)
            return -EAGAIN;
        
        printk(KERN_DEBUG "%i(%s) writer is going to sleep\n", current->pid, current->comm);

        prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
        if (spacefree(dev) == 0)
            schedule();

        finish_wait(&dev->outq, &wait);

        if (signal_pending(current))
            return -ERESTARTSYS;
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
    }
    return 0;
}

ssize_t scullpipe_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scullpipe_dev *dev = filp->private_data;
    int result = 0;
    
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
        
    result = scullpipe_getwritespace(dev, filp);     
    if (result)
        return result;

    count = min(count, (size_t)spacefree(dev));
    if (dev->wp >= dev->rp)
        count = min(count, (size_t)(dev->end - dev->wp));
    else 
        count = min(count, (size_t)(dev->rp - dev->wp - 1));
    printk(KERN_DEBUG "Going to accept %li bytes to %p from %p\n", 
                count, dev->wp, buf);
    
    if (copy_from_user(dev->wp, buf, count)) {
        up(&dev->sem);
        return -EFAULT;
    }

    dev->wp += count;
    if (dev->wp == dev->end)
        dev->wp = dev->buffer;
    up(&dev->sem);

    wake_up_interruptible(&dev->inq);
    printk(KERN_DEBUG "%i(%s) did write %li bytes\n", current->pid, current->comm, count);
    return count;
}


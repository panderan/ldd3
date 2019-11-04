#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include "scull_fops.h"


/* 实现相关的系统调用 */
struct file_operations scull_fops = {
	.owner =    THIS_MODULE,
	.llseek =   scull_llseek,
	.read =     scull_read,
	.write =    scull_write,
	.open =     scull_open,
	.release =  scull_release,
    .ioctl =    scull_ioctl,
};


int scull_trim(struct scull_dev *dev)
{
    struct scull_qset *next = NULL, *dptr = NULL;
    int qset = dev->qset;
    int i = 0;
    
    for (dptr = dev->data; dptr; dptr = next) {
        if (dptr->data) {
            for (i=0; i<qset; i++) {
                kfree(dptr->data[0]);
            }
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }

    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
    return 0;
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev = NULL;

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        if (down_interruptible(&dev->sem)) {
            return -ERESTARTSYS;
        }
        scull_trim(dev);
        up(&dev->sem);
    }
    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    return 0;
}

struct scull_qset *scull_follow(struct scull_dev *dev, int n) 
{
    struct scull_qset *qs = dev->data;

    if (!qs) {
        qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (qs == NULL) {
            return NULL;
        }
        memset(qs, 0, sizeof(struct scull_qset));
    }

    while (n--) {
        if (!qs->next) {
            qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (qs->next == NULL) {
                return NULL;
            }
            memset(qs->next, 0, sizeof(struct scull_qset));
        }
        qs = qs->next;
        continue;
    }

    return qs;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr = NULL;
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset;
    int item = 0, s_pos = 0, q_pos = 0, rest = 0;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }
    if (*f_pos >= dev->size) {
        goto out;
    }
    if (*f_pos + count > dev->size) {
        count = dev->size - *f_pos;
    }

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;
    
    dptr = scull_follow(dev, item);
    if (dptr == NULL || dptr->data == NULL || dptr->data[s_pos] == NULL) {
        goto out;
    }
    
    if (count > quantum - q_pos) {
        count = quantum - q_pos;
    }

    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

out:
    up(&dev->sem);
    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr = NULL;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset;
    int item = 0, s_pos = 0, q_pos = 0, rest = 0;
    ssize_t retval = -ENOMEM;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }
    
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum; 

    dptr = scull_follow(dev, item);
    if (dptr == NULL) {
        goto out;
    }
    if (dptr->data == NULL) {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (dptr->data == NULL) {
            goto out;
        }
        memset(dptr->data, 0, qset * sizeof(char *));
    }

    if (dptr->data[s_pos] == NULL) {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (dptr->data[s_pos] == NULL) {
            goto out;
        }
    }

    if (count > quantum - q_pos) {
        count = quantum - q_pos;
    }
    
    if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    if (dev->size < *f_pos) {
        dev->size = *f_pos;
    }

out:
    up(&dev->sem);
    return retval;
}

loff_t scull_llseek(struct file *filp, loff_t off, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t newpos;

    switch(whence) {
        case 0: /* SEEK_SET */
            newpos = off;
            break;
        case 1: /* SEEK_CUR */
            newpos = filp->f_pos + off;
            break;
        case 2: /* SEEK_END */
            newpos = dev->size + off;
            break;
        default:
            return -EINVAL;
    }

    if (newpos < 0) {
        return -EINVAL;
    }
    filp->f_pos = newpos;
    return newpos;
}

int scull_ioctl(struct inode *inode, struct file *filp,
                unsigned int cmd, unsigned long arg)
{
    int err = 0, tmp = 0;
    int retval = 0;

    if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;

    if (_IOC_DIR(cmd) & _IOC_READ) {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE) {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (err) {
        return -EFAULT;
    }

    switch(cmd) {
        case SCULL_IOCRESET:
            scull_quantum = SCULL_QUANTUM;
            scull_qset = SCULL_QSET;
            break;
        
        /* 通过指针或值设置 scull_quantum */
        case SCULL_IOC_S_QUANTUM:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            retval = __get_user(scull_quantum, (int __user *)arg);
            break;
        case SCULL_IOC_T_QUANTUM:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            scull_quantum = arg;
            break;

        /* 通过指针或值读取 scull_quantum */
        case SCULL_IOC_G_QUANTUM:
            retval = __put_user(scull_quantum, (int __user *)arg);
            break;
        case SCULL_IOC_Q_QUANTUM:
            return scull_quantum;

        /* 通过指针或值设置 scull_quantum 并返回旧值 */
        case SCULL_IOC_X_QUANTUM:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            tmp = scull_quantum;
            retval = __get_user(scull_quantum, (int __user *)arg);
            if (retval == 0)
                retval = __put_user(tmp, (int __user *)arg);
            break;
        case SCULL_IOC_H_QUANTUM:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            tmp = scull_quantum;
            scull_quantum = arg;
            return tmp;

        /* 通过指针或值设置 scull_qset */
        case SCULL_IOC_S_QSET:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            retval = __get_user(scull_qset, (int __user *)arg);
            break;
        case SCULL_IOC_T_QSET:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            scull_qset = arg;
            break;

        /* 通过指针或值读取 scull_qset */
        case SCULL_IOC_G_QSET:
            retval = __put_user(scull_qset, (int __user *)arg);
            break;
        case SCULL_IOC_Q_QSET:
            return scull_qset;

        /* 通过指针或值设置 scull_qset 并返回旧值 */
        case SCULL_IOC_X_QSET:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            tmp = scull_qset;
            retval = __get_user(scull_qset, (int __user *)arg);
            if (retval == 0)
                retval = __put_user(tmp, (int __user *)arg);
            break;
        case SCULL_IOC_H_QSET:
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            tmp = scull_qset;
            scull_qset = arg;
            return tmp;

        default:
        return -ENOTTY;
    }
    return retval;
}


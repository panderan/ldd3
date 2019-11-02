#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include "proc_seq.h"
#include "scull.h"

void * scull_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= scull_nr_devs) {
        return NULL;
    }
    return scull_devices + *pos;
}

void * scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= scull_nr_devs) {
        return NULL;
    }
    return scull_devices + *pos;
}

void scull_seq_stop(struct seq_file *s, void *v)
{
}

int scull_seq_show(struct seq_file *s, void *v)
{
    struct scull_dev *dev = (struct scull_dev *)v;
    struct scull_qset *d = NULL;
    int i = 0;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    seq_printf(s, "\nDevice %i: qset: %i, q :%i, sz: %li\n",
                (int)(dev-scull_devices), dev->qset, dev->quantum, dev->size);
    for (d=dev->data; d; d=d->next) {
        seq_printf(s, "  item at %p, qset at %p\n", d, d->data);
        if (d->data && !d->next) {
            for (i=0; i<dev->qset; i++) {
                if (d->data[i]) {
                    seq_printf(s, "    %4i: %8p\n", i, d->data[i]);
                }
            }
        }
    }
    up(&dev->sem);
    return 0;
}

struct seq_operations scull_seq_ops = {
    .start = scull_seq_start,
    .next = scull_seq_next,
    .stop = scull_seq_stop,
    .show = scull_seq_show
};

int scull_proc_open(struct inode *inode, struct file *filp)
{
    return seq_open(filp, &scull_seq_ops); 
}

struct file_operations scull_proc_ops = {
    .owner = THIS_MODULE,
    .open = scull_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};

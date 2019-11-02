#include <linux/proc_fs.h>
#include "scull.h"
#include "proc.h"
#include "proc_seq.h"

int scull_read_procmem(char *buf, char **start, off_t offset, \
                            int count, int *eof, void *data)
{
    int i = 0, j = 0, len = 0;
    int limit = count - 80;

    for (i = 0; i < scull_nr_devs && len <= limit; i++) {
        struct scull_dev *d = &scull_devices[i];
        struct scull_qset *qs = d->data;
        len += sprintf(buf+len, "\nDevice %i: qset %i, q %i, sz %li\n",
                        i, d->qset, d->quantum, d->size);
        for(; qs && len <= limit; qs = qs->next) {
            len += sprintf(buf+len, "item at %p, qset at %p\n",
                            qs, qs->data);
            if (qs->data && !qs->next) {
                for (j = 0; j < d->qset; j++) {
                    if (qs->data[j]) {
                        len += sprintf(buf + len, "    %4i: %8p\n", j, qs->data[j]);
                    }
                }
            }
        }
    }
    *eof=1;
    return len;
}

void scull_create_proc(void)
{
    struct proc_dir_entry *entry = NULL;

    // Proc 接口
    create_proc_read_entry("scullmem", 0,
                            NULL,
                            scull_read_procmem,
                            NULL);
    // Seq file 接口
    entry = create_proc_entry("scullseq", 0, NULL);
    if (entry) {
        entry->proc_fops = &scull_proc_ops;
    }
    
    return;
}

void scull_remove_proc(void)
{
    remove_proc_entry("scullmem", NULL);
    remove_proc_entry("scullseq", NULL);
}


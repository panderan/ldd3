#ifndef _SCULLPIPE_H_
#define _SCULLPIPE_H_

#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/wait.h>

#ifndef SCULLPIPE_MAJOR
#define SCULLPIPE_MAJOR 0       /* 主设备号，0 - 动态分配 */
#endif

#ifndef SCULLPIPE_NR_DEVS
#define SCULLPIPE_NR_DEVS 2     /* 设备数量，SCULLPIPE0 到 SCULLPIPE1 */
#endif

struct scullpipe_dev {
    wait_queue_head_t inq;
    wait_queue_head_t outq;
    char *buffer, *end;
    int buffersize;
    char *rp, *wp;
    int nreaders, nwriters;
    struct semaphore sem;
	struct cdev cdev;	     /* 字符设备结构	*/
};

extern struct scullpipe_dev *scullpipe_devices;

extern int scullpipe_major;
extern int scullpipe_minor;
extern int scullpipe_nr_devs;

int scullpipe_require_devnum();
void scullpipe_setup_cdev(struct scullpipe_dev *dev, int index);
int scullpipe_create();

#endif /* _SCULLPIPE_H_ */

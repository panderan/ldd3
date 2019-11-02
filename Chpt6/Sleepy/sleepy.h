#ifndef _SLEEPY_H_
#define _SLEEPY_H_

#include <linux/cdev.h>
#include <linux/wait.h>

#ifndef SLEEPY_MAJOR
#define SLEEPY_MAJOR 0       /* 主设备号，0 - 动态分配 */
#endif

#ifndef SLEEPY_NR_DEVS
#define SLEEPY_NR_DEVS 2     /* 设备数量，sleepy0 到 sleepy1 */
#endif


struct sleepy_dev {
    int flag;
    wait_queue_head_t my_queue;
	struct cdev cdev;	     /* 字符设备结构	*/
};

#endif /* _SLEEPY_H_ */

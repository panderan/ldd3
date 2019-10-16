#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/string.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Ben");
MODULE_VERSION("1.0");


static char *whom = "world";
static int howmany = 1;


module_param(howmany, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);


void print_current_process_info(void);


static int __init hello_init(void)
{
    int i = 0;
    for (i=0; i<howmany; i++) {
        printk(KERN_ALERT "%s\n", whom);
    }
    print_current_process_info();
    return 0;
}

void print_current_process_info(void)
{
    printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm, current->pid);
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);

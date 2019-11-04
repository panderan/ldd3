/* 内核模块的基本头文件 */
#include <linux/module.h>
#include <linux/init.h>

#include "scullpipe.h"
#include "sc_pipe_fops.h"

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");


/// 模块清除函数
/**
  * 清除函数可以被用来处理初始化失败的场景，因此，它必须能够正确的处理一些还没
  * 被初始化的地方。
  */
void scullpipe_cleanup_module(void)
{
    scullpipe_destroy();
}


int scullpipe_init_module(void)
{
	int result = 0;

    if((result=scullpipe_create()) != 0) {
        goto fail;
    }
	return 0;

fail:
	scullpipe_cleanup_module();
	return result;
}

module_init(scullpipe_init_module);
module_exit(scullpipe_cleanup_module);


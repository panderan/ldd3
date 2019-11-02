## 第四章 并发和竞态

**Changelog：**

在第四章 Scull 代码的基础上增加并发控制。scull_dev 设备结构体是一个共享资源，当有多个进程访问该设备时如果不进行并发控制则会产生无法预期的结果。因此本节利用信号量 semaphore 对设备进行互斥访问。

-   scull_dev 增加 struct semaphore sem 数据项；

**知识脑图**

![并发与竞态知识脑图](../images/readme-chpt5.jpg)


## ThreadPool

### 项目描述

* 基于可变参模板和引用折叠原理，实现线程池submitTask接口，支持任意函数和任意参数的传递
* 使用future类型定制submitTask提交任务的返回值
* 使用map和queue容器管理线程对象和任务
* 基于条件变量condition_variable和互斥锁mutex实现任务提交线程和任务执行线程间的通信机制
* 支持fixed和cached模式的线程池定制


### 开发中的问题

* package_task打包的任务传入任务队列，必须是按值传入，不然在任务真正执行时，package_task对象已经被释放了，通过future获取返回值时发生空指针引用。
* 在TrehadPool的资源回收，等待线程队列退出时，发生死锁问题，导致进程无法退出。

开发语言：C++

背景：构建高并发服务器程序，实现10万次客户端并发连接 

架构： Epoll的ET模式+多线程+mysql数据库

编译：make clean;make;make stress_client

	server:服务器可执行文件

	stress_client:压测的客户端执行文件

状态：只写了I/O处理模块，业务模块在开发，欢迎感兴趣的，或者想技术进阶的一起来提交代码


问题：目前代码里支持连接10K客户端连接但是数据处理有问题；内核参数如何微调？
	  请求队列与多线程如何高效率协同工作？如何高效率的数据库操作

tcp_config.md：主要是配置服务器内核参数

邮箱：有问题可以直接发邮件到hyellowriver@gmail.com，或者QQ824722455


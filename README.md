# 引子

网络编程本身是一件有意思的事情, 尽管网络的基础概念, 协议比较多. 好在平时工作中用到的最多的还是tcp/ip协议栈.

如果你不是专攻网络的专家, 只是一般的研发或者网络开发, 对网络知识和经验有一定的要求, 但是没有太多要求.

本教程, 也是我学习网络编程(base on c/c++)的过程, 一个轻松愉快的过程.
(由于是把以前的知识整合归纳, 然后输出, 所有难免会有一些个人的看法, 见谅)

(不要学的很痛苦, 不要文章太长)

除了教程类, 您还可以关注我的github账号和github博客, 这里也许有你想要的:

1. github: https://github.com/WizardMerlin
   (分享easy的教程, 分享成熟的库, 自己的轮子)
2. 私人博客: https://wizardmerlin.github.io/
   (主要是概括性的文章, 思考, 书评, 翻译, 深入剖析)
3. 社区博客:(讨论开源, 提问 & 回答别人的问题)
   * https://my.oschina.net/wizardmerlin/blog  (讨论 & 简单的教程)
   * https://segmentfault.com/u/wizardmerlin  (问答 & 简单的教程)

   当然也欢迎你加入我们的日常组, 高级Linux开发群: `59151639`


其他技术问题, 只要我知道或者感兴趣,一定会个给您满意的回复.
  
期待更多的交流: `wizardmerlin945@gmail.com`





# 正文


## 网络学习步骤

有基础的1个月; 没有基础的2.5个月. 

深入学习还需3个月.

(本文建立在你熟悉一门支持网络编程的计算机语言的基础上; 最好对于网络并发,多线程; Linux系统有一定的了解)

1. TCPIP模型 (网络的理论基础)
2. 学习socket api集.
   * 创建 socket bind listen accept
   * 收发 read/recv/recvfrom write/send/sendto
   * 关闭 close shutdown
   * 参数 getsockopt/setsockopt
   * 地址 gethostbyaddr getaddrbyhost,...
在学习这些api时候，可以先关注在函数功能，参数意义上
3. 基本的socket编程模型(socket api的理论实践)
   * tcp socket的语法如下: socket(AF_INET, SOCK_STREAM)
   * udp socket的语法如下: socket(AF_INET, SOCK_DGRAM)
4. 学习实现server端编程, 关注服务器
学习socket server端编程实现简单规约比如echo, time等, 然后通过cmd中的telnet来测试.
5. 学习相关网络IO模型()
演化过程, 比如阻塞, 非阻塞, 异步, 事件IO(select,poll,epoll, kqueue, WaitForMultipleObject)等
6. 学习Richard Stevens的<Unix网络编程>, 深入学习其中的api原理以及服务端设计原理, 并通过代码编写
7. 学习高性能网络框架libevent, libuv, boost.asio等
8. 把所学知识或者经验用于工业/商业项目


以上均为个人的浅见, 不妥之处, 还请指点.




## 个人笔记





# 尾巴


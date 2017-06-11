IO函数总结

尽管别的文件已经说了, 但是这里还是进行集中总结一下


网络IO操作:(最常见的, 分组)

1. read()/write() ---简单的使用Linux的文件api :

```c++
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
```

介绍一下read, write:
        read函数是负责从 fd中读取内容.
	当读成功时, read返回实际所读的字节数(>0), 如果返回的值是0表示已经读到文件的结束了, 小于0表示出现了错误.
	如果错误为 EINTR 说明读是由中断引起的, 如果是ECONNREST表示网络连接出了问题.


	write函数将buf中的nbytes字节内容写入文件描述符fd.成功时返回写的字节数(>0).失败时返回-1, 并设置 errno变量.
	在网络程序中, 当我们向套接字文件描述符写时有俩种可能:
	1. 返回值大于0; 表示写了部分或者是全部的数据
	2. 返回值小于0; 此时出现了错误.
	错误为EINTR 表示在写的时候出现了中断错误
	如果为EPIPE表示网络连接出现了问题(对方已经关闭了连接)


2. 专门用于socket的:
	recv()/send()----和read, write大致一样, 不过flags提供了更加强大的选项(flags一般设置为0)
			 
	flags其他可以取得的值:
	  * MSG_OOB 接收以 out-of-band 送出的数据
	  * MSG_PEEK 返回来的数据并不会在系统内删除, 如果再调用recv()会返回相同的数据内容
	  * MSG_WAITALL 强迫接收到 len 大小的数据后才能返回, 除非有错误或信号产生
	  * MSG_NOSIGNAL 此操作不愿被 SIGPIPE 信号中断返回值成功则返回接收到的字符数, 失败返回-1, 错误原因存于 errno 中

			  
	recvfrom()/sendto() ---- 一般用在udp中(server bind之后之后不需要listen直接和客户端进行通信)---容易阻塞
	recvmsg()/sendmsg() ---- 最通用的, 最强大的,可以实现前面所有函数的功能.(涉及到的struct msghdr, struct iovec比较复杂)

	如果利用 UDP 协议则不需经过连接操作;  
	参数 msg 指向欲连线的数据结构内容, 参数 flags 一般设 0 .  (具体用起来比较复杂)


相关函数原型如下:

```c++
	
#include <sys/types.h>
#include <sys/socket.h>
								  
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
	       const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
		 struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);

struct msghdr
{
  void *msg_name;
  int msg_namelen;
  struct iovec *msg_iov;
  int msg_iovlen;
  void *msg_control;
  int msg_controllen;
  int msg_flags;
}

  struct iovec
  {
    void *iov_base; /* 缓冲区开始的地址 */
    size_t iov_len; /* 缓冲区的长度 */
  }

```


3. 关闭IO

```c++
  int close(int fd);
```


close 操作只是使相应socket描述字的引用计数-1, 只有当引用计数为0的时候, 才会触发TCP客户端向服务器发送终止连接请求.
  (被关闭的套接字不能在用于read或者write了; 注意你关闭了是accept出来用于响应客户端读写请求的socket fd)

  int shutdown(int sockfd, int howto)
  TCP 连接是双向的(是可读写的),当我们使用 close 时,会把读写通道都关闭,有时侯我们希望只关闭一个方向,这个时候我们可以使用 shutdown.
  针对不同的 howto,系统回采取不同的关闭方式.
  howto=0 这个时候系统会关闭读通道.但是可以继续往接字描述符写.
  howto=1 关闭写通道,和上面相反,着时候就只可以读了.
  howto=2 关闭读写通道,和 close 一样 
  
  close() Vs shutdown()
  在多进程程序里面, 如果有几个子进程共享一个套接字时, 如果我们使用 shutdown, 那么所有的子进程都不能够操作了.
  这个时候我们只能够使用 close 来关闭子进程的套接字描述符.



4. ioctl

函数原型如下:
```c++
#include <sys/ioctl.h>
int ioctl(int d, unsigned long request, ...);
```

(参考 man ioctl_list) (list of ioctl calls in Linux/i386 kernel)

控制的内容太多了(socket fd也可以当做一个文件), 这里主要说明控制的套接字选项: 
    1. SIOCATMARK  是否到达带外标记 int
    2. FIOASYNC  异步输入/输出标志 int
    3. FIONREAD 缓冲区可读的字节数 int


5. fcntl

函数原型如下:
```c++
       #include <unistd.h>
       #include <fcntl.h>

       int fcntl(int fd, int cmd, ... /* arg */ );
```

一般用于设置非阻塞IO. 
(当然如果是非socket fd时, 可以使用open或者reopen, 在打开文件的时候指定非阻塞IO)

标准的做法:

```c++
  int flag=0;

  flag = fcntl(connectfd, F_GETFL);
  flag |= O_NONBLOCK;
  fcntl(connectfd, F_SETFL, flag);
```

当然简便的做法

`fcntl(connectfd, F_SETFL, O_NONBLOCK);`





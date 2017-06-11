# Socket函数集合

整理socket编程中使用到的相关函数:


1. accept       接受连接请求
2. bind         对socket定位
3. connect      建立socket连接
4. endprotoent  结束网络协议数据的读取
5. endservent   结束网络服务数据的读取
6. getsockopt   取得socket状态
7. htonl        将32位主机字节序转换成网络字节序(long)
8. htons        将16位主机字节序转换成网络字节序(short)
9. inet_addr    将网址地址转换成二进制的数字
10. inet_aton   将网络地址串转换成网络二进制的数字
11. inet_ntoa   将网络地址二进制数字转换成网络地址字符串
12. listen      等待连接
13. ntohl       将32位网络字符顺序转换成主机字符顺序
14. ntohs       将16位网络字符顺序转换成主机字符顺序
15. recv        经socket接收数据(1)
16. recvfrom    经socket接收数据(2)
17. recvmsg     经socket接收数据(3)
18. send        经socket传送数据(1)
19. sendmsg     经socket传送数据(2)
20. sendto      经socket传送数据(3)
21. setprotoent 打开网络协议的数据文件
22. setservent  打开主机网络服务的数据文件
23. setsockopt  设置socket状态
24. shutdown    终止socket通信(特别是半连接状态)
25. socket      建立一个socket通信



下面按照头文件相关, 进行具体的叙述.

---


Socket模型中必备的头文件:

1. 和基本Socket函数相关的头文件

```c++
#include <sys/types.h>
#include <sys/socket.h>
```

socket-->bind-->listen-->accept
socket-->connect



2. 数字和字符串转换

```c++
#include <stdlib.h>
int atoi(const char *nptr);
long atol(const char *nptr);
```


3. 地址端口号转换(数字字节转换)

ip地址的转换(server_addr.sin_addr.s_addr) :  htonl
端口的转换(server_addr.sin_port) : htons

本机数字类型转换到网络数据类型--按字节转换
( host, network, long, short )

```c++
#include <arpa/inet.h>
  
uint32_t htonl(uint32_t hostlong);  //uint32_t: unsigned long 
uint16_t htons(uint16_t hostshort); //uint16_t: unsigned short

uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```


5.  字符串地址和网络地址(二进制)  ---直接从字符串到网络序 (更加方便了)
 array <--> net
```c++
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int inet_aton(const char *cp, struct in_addr *inp); //重点 "a.b.c.d" ---> 32位ip
char *inet_ntoa(struct in_addr in);  //重点 32位ip --->  "a.b.c.d" 
```
可以直接用 `server_addr.sin_addr` , 而不是 `server_addr.sin_addr.s_addr`

下面是补充:
网络地址字符串是以数字和点组成的字符串, 例如: “163.13.132.68”
  
```c++
in_addr_t inet_addr(const char *cp);  //网络字符串转换成二进制数字(unsigned long int), 
in_addr_t inet_network(const char *cp);

struct in_addr inet_makeaddr(in_addr_t net, in_addr_t host);

in_addr_t inet_lnaof(struct in_addr in);
in_addr_t inet_netof(struct in_addr in);
```


  
6. 域名和ip地址(网络序)转换

```c++
#include <netdb.h>
  
extern int h_errno;
struct hostent *gethostbyname(const char *name);

#include <sys/socket.h>       /* for AF_INET */
struct hostent *gethostbyaddr(const void *addr,
			      socklen_t len, int type);
```

这两个函数失败时返回 NULL 且设置 h_errno 错误变量, 调用 h_strerror()可以得到详细的出错信息

关于 `hostent struct` ...
```c++
//The hostent structure is defined in <netdb.h> as follows:
  struct hostent {
    char  *h_name;            /* official name of host */
    char **h_aliases;         /* alias list */
    int    h_addrtype;        /* host address type */
    int    h_length;          /* length of address 对于 IP4 是 4 字节 32 位*/
    char **h_addr_list;       /* list of addresses */
  }
#define h_addr h_addr_list[0] /* for backward compatibility */

```

例如:(假设提前定义了 `struct hostent *`)

```c++
/* 客户程序填充服务端的资料 */
bzero(&server_addr,sizeof(server_addr));
server_addr.sin_family=AF_INET;
server_addr.sin_port=htons(portnumber);
//watch out here, 实际取得的是首地址元素
server_addr.sin_addr=*((struct in_addr *)host->h_addr);   
```

其中 `*((struct in_addr *)host->h_addr);` 等价于 `addr.sin_addr=*(struct in_addr *)(host->h_addr_list[0])`;
所以直接取到了网络地址 `*((struct in_addr *)host->h_addr)` 可以赋值给server_addr.sin_addr.

再例如:
本来输入 "a.b.c.d", 结果输入"localhost"

``c++

/*deal with addr.sin_addr*/
if(inet_aton(argv[1], &addr.sin_addr)==0) { /*what if user input 'localhost'*/
  host=gethostbyname(argv[1]);
  
  if(host==NULL) {
    fprintf(stderr,"HostName Error:%s\n\a",hstrerror(h_errno));
    exit(1);
  }

  //*((struct in_addr *)host->h_addr); //the same
  addr.sin_addr=*(struct in_addr *)(host->h_addr_list[0]);
}

```

相关的还有:

```c++
#include <netdb.h>
void setprotoent (int stayopen);
void setservent (int stayopen);
```

setprotoent()用来打开/etc/protocols; setservent()用来打开/etc/services;
如果参数 stayopen 值为 1, 则接下来的 getprotobyname() 或 getprotobynumber() 将不会自动关闭此文件.



7. 其他信息函数(用的比较少)

当我们要得到连接的端口号时, 在connect调用成功后使用, 可得到系统分配的端口号
(客户端的端口号由操作系统默认绑定)
```c++
#include <sys/socket.h>

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

对于服务端, 我们用 INADDR_ANY 填充后, 为了得到连接的IP. 在 accept调用成功后使用而得到IP地址结构体,
之后借助该函数可以拿到对端socket fd.
```c++
#include <sys/socket.h>

/*returns  the  address of the peer connected to the socket sockfd, in the buffer pointed to by addr.
  The addrlen argument should be initialized to indicate the amount of space pointed to by addr.
*/
int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

下面的函数得到端口号或者服务号:

```c++
#include <netdb.h>
  
struct servent *getservbyname(const char *name, const char *proto); 
struct servent *getservbyport(int port, const char *proto);
    
struct servent
{
      char *s_name; /* 正式服务名 */
      char **s_aliases; /* 别名列表 */
      int s_port; /* 端口号 */
      char *s_proto; /* 使用的协议 */
}
```



8. 错误处理

```c++
#include <string.h>

char *strerror(int errnum);

int strerror_r(int errnum, char *buf, size_t buflen);
/* XSI-compliant */

char *strerror_r(int errnum, char *buf, size_t buflen);
/* GNU-specific */

char *strerror_l(int errnum, locale_t locale);
```

例如: 
 `fprintf(stderr,"Listen error:%s\n\a",strerror(errno))`



9. 初始化

比memset还好用的bzero.
```c++
#include <strings.h>
void bzero(void *s, size_t n);
```

The bzero() function sets the first n bytes of the area starting at s to zero (bytes containing '\0').



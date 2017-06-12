#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_EVENTS 1024
#define BUFLEN 4096
#define SERV_PORT 8080


/*读完换成写事件监听*/
void recvdata(int fd, int events, void *arg) 
{
  struct myevent_s *ev = (strcut myevent_s *) arg;
  int len;

  len = recv(fd, ev->buf, sizeof(ev->buf), 0); //存储myevet_s中的buf成员中
  eventdel(g_efd, ev); //将该节点从红黑树上摘除 (具体操作之前先摘除, 并且下面改成监听EPOLLOUT)

  if(len>0) {
    ev->len = len;
    ev->buf[len]='\0'; //方便输出
    printf("C[%d]:%s\n", fd, ev->buf);

    eventset(ev, fd, senddata, ev); //将fd对应的回调函数设置为senddata
    eventadd(g_efd, EPOLLOUT, ev);  //将fd加入红黑树g_efd中, 监听其写事件
  } else if (len==0) {
    close(ev->fd);
    printf("[fd=%d] pos[%ld], closed\n", fd, ev - g_events); //地址偏移(g_events是数组首地址)
  } else {
    close(ev->fd);
    printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
  }
  
  return;
}


/*写完变成读监听*/
void senddata(int fd, int events, void *arg) 
{
  struct myevent_s *ev = (struct myevent_s *)arg;
  int len;
  
  len = send(fd, ev->buf, ev->len, 0); /*写完改成监听读*/

  if(len >0) {

    printf("send[fd=%d], [%d] %s\n", fd, len, ev->buf);
    eventdel(g_efd, ev);
    eventset(ev, fd, recvdata, ev);
    eventadd(g_efd, EPOLLIN, ev); /*该为读监听*/

  } else {

    close(ev->fd);
    evetdel(g_efd, ev);
    printf("send[fd=%d] error %s\n", fd, strerror(errno));

  }

  return;
}



/*描述就绪文件描述符相关信息*/

struct myevent_s {
  int fd;  //需要监听的描述符
  int events; //对应的监听事件
  void *arg;  //注意该泛型指针指向本结构体

  void (*call_back)(int fd, int events, void *arg);
  int status; //是否在红黑树上(监听)
  char buf[BUFLEN];
  int len;
  long last_active; //记录每次加入红黑树的时间值(踢掉不活跃的连接)
};


int g_efd; //红黑树的树根
struct myevent_s g_events[MAX_EVENTS+1]; //自定义事件(监听fd数组): +1表示listen fd


/*将结构体 myevent_s 成员变量 初始化*/
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void*), void *arg)
{
  ev->fd = fd;
  ev->call_back = call_back;
  ev->events = 0;
  ev->arg = arg;
  ev->status = 0;
  memset(ev->buf, 0, sizeof(ev->buf));
  ev->len = 0;
  ev->last_active = time(NULL); //调用eventset函数的时间

  return;
}

/*向epoll监听的红黑树 添加一个 文件描述符*/
void eventadd(int efd, int events, struct myevent_s *ev) 
{
  struct epoll_event epv = {0, {0}};
  int op;
  epv.data.ptr = ev;
  epv.events = ev->events = events; //EPOLLIN或者EPOLLOUT

  if(ev->status == 1) { //已经在树上
    op = EPOLL_CTL_MOD;
  }else {
    op = EPOLL_CTL_ADD;
    ev->status = 1;
  }
    
  //实际添加或者修改
  if( (epoll_ctl(efd, op, ev->fd, &epv)) <0 ) {
    printf("event add failed [fd=%d]\n", ev->fd, events);
  } else {
    printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, events);
  }
  
  return;
}


/*从epoll监听的红黑树上删除一个文件描述符(epoll_event)*/
void eventdel(int efd, struct myevent_s *ev)
{
  struct epoll_event epv = {0, {0}};

  if(ev->status != 1) {
    return ;
  }

  epv.data.ptr = ev;
  ev->status = 0;
  epoll_ctl(efd, EPOLL_CTL_DEL, ev-fd, &epv);

  return;
}

/*当有描述符就绪, epoll返回, 调用该函数与客户端建立连接*/
void acceptconn(int lfd, int events, void *arg)
{
  struct sockaddr_in cin;
  socklen_t len = sizeof(cin);

  int cfd, i;

  if((cfd=accept(lfd, (struct sockaddr *)&cin, &len)) == -1) {
    if(errno != EAGAIN && errno != EINTR){
      //不做处理
    }
    printf("%s: accept, %s\n", __func__, strerror(errno));
    return;
  }

  do {
    for (i = 0; i<MAX_EVENTS; ++i){ //g_events数组中找个空位置放入cfd
      if(g_events[i].status = 0) { //该位置还没有被监测
	break;
      }
      
      if(i == MAX_EVENTS) {
	printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
	break; //后续代码就不在执行了
      }
      
      int flag = 0;
      if((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) { //将cfd设置为阻塞
	printf("%s: fcntl nonblocking failed, %s \n", __func__, strerror(errno));
	break;
      }
      /*为cfd设置一个myevent_s结构体, 回调函数 设置为recvdata*/
      eventset(&g_event[i], cfd, recvdata, &g_event[i]); //填充g_event[i]
      eventdadd(g_efd, EPOLLIN, &g_events[i]);//将cfd挂到g_efd红黑树上
      
    }//end of for
  } while (0);

  printf("new connect [%s:%d][time:%ld], pos[%d]\n", inet_ntoa(cin.sin_addr), 
	 ntohs(cin.sin_port), g_events[i].last_active, i);
  return;  
}

void initlistensocket(int efd/*树根*/, short port)
{
  int lfd = socket(AF_INET, SOCK_STREAM, 0);

  fcntl(lfd, F_SETFL, O_NONBLOCK); //监听套接字设置为非阻塞

  /* void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void*), void *arg); */
  //相当于epoll_ctl的封装, 主要用来封装struct myevent_s *这个传出参数
  eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]); /*最后一个参数指向结构体本身*/


  /* void eventadd(int efd, int events, struct myevent_s *ev) */
  //用后两个传入参数events和void*ptr来填充epoll_event并把它加入rbtree
  eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);
  
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port = htons(port);

  bind(lfd, (struct sockaddr *)&sin, sizeof(sin));
  listen(lfd, 20);

  return;  
}




int main(int argc, char *argv[]) 
{
  unsigned short port = SERV_PORT;
  if (argc == 2 ) {
    port = atoi(argv[i]); //使用用户指定端口, 如果没有指定, 则使用默认
  }

  g_efd = epoll_create(MAX_EVENTS+1); //创建红黑树, 返回给全局g_efd
  if (g_efd <= 0) {
    printf("create efd in %s err %s\n", __func__, strerror(errno));
  }

  /*初始化监听*/
  initlistensocket(g_efd, port);

  struct epoll_event events[MAX_EVENTS+1]; //保存ready的描述符(就绪)
  printf("server running:port[%d]\n", port);

  int checkpos = 0/*每次测试100个连接*/, i/*循环计数*/;
  while(1) {
    /*超时验证, 当客户端60秒内没有和服务器通信, 则关闭此客户端的连接;
      不测试listenfd
      每次测试100个连接, 依次往下
     */
    long now = time(NULL);
    for(i=0; i<100; i++, checkpos++) {
      if(checkpos == MAX_EVENTS) {
	checkpos = 0;
      }
      if(g_events[checkpos].status !=1 ) { //不在红黑树上
	continue;
      } 
	
      long duration = now - g_events[checkpos].last_active;
      if(duration >= 60) {
	close(g_events[checkpos].fd); //关闭客户端连接
	printf("[fd=%d] timeout\n", g_events[checkpos].fd);
	eventdel(g_efd, &g_events[checkpos]); //从监听红黑树上摘除
      }

    }//end of for(i)

    //1秒超时, events为满足要求的事件数组, nfd为数量
    int nfd = epoll_wait(g_efd, events, MAX_EVENTS+1, 1000);
    if(nfd < 0) {
      printf("epoll_wait error, exit\n");
      break;
    }

    /*遍历ready集合*/
    for(i=0; i<nfd; i++) {
      /*ev指针接收epoll_event.data的泛型指针ptr*/
      /*初始化的时候把该指针指向了自定义结构体*/
      struct myevent_s *ev = (struct myevent_s *) events[i].data.ptr;

      if((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) { //ready read
	/*一些相关操作*/
	ev->call_back(ev->fd, events[i].events, ev->arg);
      }
      if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) { //reay write
	/*一些相关操作*/
	ev->call_back(ev->fd, events[i].events, ev->arg);
      }
    }

    
  }//end of while(1)

  
  /*释放资源release*/
  //


  return;


}


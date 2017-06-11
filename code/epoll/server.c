#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>

#include "wrap.h"

#define MAXLINE 8192
#define SERV_PORT 6666
#define OPEN_MAX 5000

int main(int argc, char *argv[])
{
  int i, listenfd, connfd, sockfd/*read-write client*/;
  int n, num = 0; /*num records the number of connected clients*/
  ssize_t nready, efd, res;
  char buf[MAXLINE], str[INET_ADDRSTRLEN]/*for client IP*/;
  socklen_t clilen;

  struct sockaddr_in cliaddr, servaddr;
  struct epoll_event tep, ep[OPEN_MAX]; //tep: epoll_ctl参数, ep[]: epoll_wait参数

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);
  
  /*端口复用*/
  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

  Listen(listenfd, 20);

  efd = epoll_create(OPEN_MAX); //开始构建红黑树
  if(efd == -1) {
    perr_exit("epoll_create error.");
  }
  
  tep.events = EPOLLIN; //指定lfd监听读事件
  tep.data.fd = listenfd;
  res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
  if(res == -1) {
    perr_exit("epoll_ctl error.");
  }

  for(;;) {
    nready = epoll_wait(efd, ep, OPEN_MAX, -1);
    if(nready == -1) {
      perr_exit("epoll_wait error.");
    }

    for(i=0; i<nready; ++i) {
      if (!(ep[i].events & EPOLLIN)) { //不是读事件, 继续循环下一个活跃的fd
	continue;
      }
      if(ep[i].data.fd == listenfd) {
	clilen = sizeof(cliaddr);
	connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
	
	printf("received from %s at PORT %d\n",
	       inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
	       ntohs(cliaddr.sin_port));
	printf("connect fd %d---client %d\n", connfd, ++num);
	
	/*add to rbtree*/
	tep.events = EPOLLIN;
	tep.data.fd = connfd;
	
	res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
	if(res == -1){
	  perr_exit("epoll_ctl error");
	}
      } else { /*alredy connected*/
	sockfd = ep[i].data.fd;
	n = Read(sockfd, buf, MAXLINE);

	if(n == 0) { //client alreay closed(remove from rbtree)
	  res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
	  if(res == -1) {
	    perr_exit("epoll_ctl error");
	  }
	  Close(sockfd);
	  printf("client[%d] closed connection\n", sockfd);
	} else if(n<0){ //read error
	  perror("read n < 0 error:");
	  res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
	  Close(sockfd);
	} else { /*read some data*/
	  for(i=0; i<n; ++i) {
	    /*process data*/
	    buf[i] = toupper(buf[i]);
	  }
	}//end of `read some data`
	
      }//end of `connected fd`
    }//end of for(nready)
    
  }//next for(;;) and epoll_wait
  

}//end of main()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include "proxy_main.h"
#include "filter.h"

#define Port 8000

int main(){
    printf("working\n");
	/*
	 *初始化socket,并绑定
	 */
	int ret;
	/* 创建socket */
	int usersockfd = socket(AF_INET, SOCK_STREAM, 0);
	int remtsockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(usersockfd < 0){
		perror("usersockfd error");
		exit(1);
	}else if(remtsockfd < 0){
		perror("remtsockfd error");
		exit(1);
	}
	/* 设置本地地址结构体 */
	struct sockaddr_in user_addr;
	bzero(&user_addr, sizeof(user_addr));
	user_addr.sin_family = AF_INET;
	user_addr.sin_port = htons(Port);
	user_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/* 套接字绑定端口 */
	ret = bind(usersockfd, (struct sockaddr*)&user_addr, sizeof(user_addr));
	if(ret != 0){
		perror("bind error");
		close(usersockfd);
		close(remtsockfd);
		exit(1);
	}
	/* 套接字进入被动监听 */
	ret = listen(usersockfd, SOMAXCONN);//SOMAXCONN由系统决定监听队列长度，一般几百
	if(ret != 0){
		perror("listen error");
		close(usersockfd);
		close(remtsockfd);
		exit(1);
	}

	/* 设置套接字为非阻塞 */
	setNonBlocking(usersockfd);

	/*
	 *使用epoll
	 */
	/* 声明epoll_wait结构体的变量，ev用于注册事件，数组指针用于回传要处理的事件 */
	struct epoll_event ev, events[20];
	int epfd = epoll_create1(0);//除了参数size被忽略外，此函数和epoll_create完全相同
	if(epfd == -1){
		perror("epoll_create");
		abort();
	}
	ev.data.fd = usersockfd;    //设置与要处理的事件相关的文件描述符
	ev.events = EPOLLIN|EPOLLET;//设置要处理的事件类型
	/* 注册epoll事件 */
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, usersockfd, &ev);
	if(ret == -1){
		perror("epoll_ctl");
		abort();
	}
	/* 添加客户端地址结构 */
	struct sockaddr_in cli_addr;
    socklen_t cli_len;
    printf("epoll\n");
	/* the event loop */
	int nfds, i, connfd, n,sockfd;
	char line[256];
	for ( ; ; ) {
        printf("outside for\n");
		nfds = epoll_wait(epfd, events, 20, 500);
		for(i = 0; i < nfds; ++i) {
            printf("inside for\n");
            /* 建立新的连接 */
			if(events[i].data.fd == usersockfd){   //有新的连接
				connfd = accept(usersockfd, (struct sockaddr*)&cli_addr, &cli_len); //accept这个连接
				//setNonBlocking(connfd);   //设置为非阻塞
				char *str1 = inet_ntoa(cli_addr.sin_addr);
				printf("accept a connection from %s\n",str1);
				/* 设置用于读操作的文件描述符， 设置用于注册read操作事件*/
				ev.data.fd = connfd;
				ev.events = EPOLLIN|EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);   //将新的fd添加到epoll的监听队列
            /* 监听到读事件 */
			} else if (events[i].events & EPOLLIN) {   //已经连接的用户，接收到数据，读socket
                sockfd = events[i].data.fd;
                printf("epoll in\n");
                read(sockfd,line,256);
                printf("%s\n",line);

                /* 设置用于注册的写操作事件，修改sockfd上要处理的事件为epollout */
                ev.data.fd=sockfd;
                ev.events=EPOLLOUT|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
                //mTrans_t *mparam;
                //mparam = calloc(1, sizeof(mTrans_t));
                //mparam->mUsocket = sockfd;
                //mparam->mRsocket = remtsockfd;
                //tpool_add_work(TransWorker, (void*)mparam);
                //free(mparam);
			}
			/* 监听到写事件,有事件发送，socket缓冲区有空间 */
			else if(events[i].events & EPOLLOUT){
                sockfd = events[i].data.fd;
                write(sockfd,line,256);

                ev.data.fd=sockfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
			}
		}//nfds for
	}//for

	/* 关闭套接字 */
	close(usersockfd);
	close(remtsockfd);
	return 0;
}

/* usersockfd向remtsockfd传送数据，http请求 */
static void* TransWorker(void* arg){
    printf("enter TransWorker \n");
    mTrans_t *pstru;
	pstru = (mTrans_t *)arg;
	char buf[BUF_SIZE];
	char ipstr[32];
	int flag = 1;
	while (recv(pstru->mUsocket, buf, sizeof(buf), 0) > 0) {
		if(flag){
			DNtoIP(buf, ipstr);            //域名转IP
			struct sockaddr_in toRemt_addr;//利用remtsockfd发送的地址
			bzero(&toRemt_addr, sizeof(toRemt_addr));
			toRemt_addr.sin_family = AF_INET;
			toRemt_addr.sin_port = htons(80);
			toRemt_addr.sin_addr.s_addr = htonl(inet_addr(ipstr));
			flag = 0;
			/* 连接服务器 connect */
			if(connect(pstru->mRsocket, (struct sockaddr*)&toRemt_addr, sizeof(toRemt_addr)) < 0){
                perror("connect error");
                exit(-1);
			}
			/* 发送给远程主机 */
			write(pstru->mRsocket, buf, sizeof(buf));
		}
	}//while(recv)

	/*
	 *循环接收远程主机返回的http响应
	 *并返回给usersockfd
	 */
    while( read(pstru->mRsocket, buf, sizeof(buf)) > 0 ){
        /* 响应传会usersockfd */
        write(pstru->mUsocket, buf,sizeof(buf));
    }
}


/* 设置socket为非阻塞 */
int setNonBlocking(int mSocket){
	int flags, s;
	//获取文件标志位
	flags = fcntl(mSocket, F_GETFL, 0);
	if(flags == -1){
		perror("fcntl");
		return -1;
	}
	//设置文件状态标志
	flags |= O_NONBLOCK;
	s = fcntl(mSocket, F_SETFL, flags);
	if(s == -1){
		perror("fcntl");
		return -1;
	}
	return 0;
}

/* 域名转Ip函数 */
char *DNtoIP(char *mString, char *ipstr){
	struct hostent *hptr;
	//char *ipstr = (char*)calloc(32,sizeof(char*));   //记得在使用后free掉ipstr
	char *substr = "Host: ";
	char *src = strstr(mString, substr) + 6;
	char *pchar = src;
	int length = 0;
	while(*pchar != '\r'){
		length++;
		pchar++;
	}
	char *domainName = (char*)malloc(length);
	strncpy(domainName, src, length);
	if((hptr = gethostbyname(domainName)) == NULL){
		perror("gethostbyname");
		return 0;
	}
	switch(hptr->h_addrtype){
		case AF_INET:
		case AF_INET6:
				inet_ntop(hptr->h_addrtype, hptr->h_addr, ipstr,sizeof(ipstr));
				break;
		default:
				perror("unknown address type\n");
				break;
	}
	free(domainName);
	return ipstr;
}

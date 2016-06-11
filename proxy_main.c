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
#define max(x,y) ((x)>(y)?(x):(y))
#define Port 8000
#define MAXBUF 1024
#define BIGBUF 8000

int main(){
    printf("working\n");
    FILE *getinfo,*getAns;
	getinfo=fopen("getInfo","w");
	getAns=fopen("getAns","w");
	int ret;
	/* 创建socket */
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0){
		perror("listenfd error");
		exit(1);
	}
	/* 设置本地地址结构体 */
	struct sockaddr_in user_addr;
	bzero(&user_addr, sizeof(user_addr));
	user_addr.sin_family = AF_INET;
	user_addr.sin_port = htons(Port);
	user_addr.sin_addr.s_addr = INADDR_ANY;

	/* 设置套接字为非阻塞 */
	setNonBlocking(listenfd);
	/* 设置端口复用 */
    int on=1;
    if((setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

	/* 套接字绑定端口 */
	ret = bind(listenfd, (struct sockaddr*)&user_addr, sizeof(user_addr));
	if(ret != 0){
		perror("bind error");
		close(listenfd);
		exit(1);
	}
	/* 套接字进入被动监听 */
	ret = listen(listenfd, SOMAXCONN);//SOMAXCONN由系统决定监听队列长度，一般几百

	if(ret != 0){
		perror("listen error");
		close(listenfd);
		exit(1);
	}





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
	ev.data.fd = listenfd;    //设置与要处理的事件相关的文件描述符
	ev.events = EPOLLIN|EPOLLET;//设置要处理的事件类型
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
	if(ret == -1){
		perror("epoll_ctl");
		abort();
	}
	/* 添加客户端地址结构 */
	struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);


    char strbuf[BIGBUF];

	/* the event loop */
	printf("epoll\n");
	int nfds, i, connfd, n,sockfd,fd,nread;
	char buf[MAXBUF];
    int index ;
	for ( index =0 ; ; ++ index) {
		nfds = epoll_wait(epfd, events, 20, 2000);
        if (nfds == -1) {
            perror("epoll_pwait");
            exit(EXIT_FAILURE);
        }
        printf("%d\n",nfds);
		for(i = 0; i < nfds; ++i) {
            printf("inside for\n");
            /* 建立新的连接 */
            fd = events[i].data.fd;
			if(fd == listenfd){   //有新的连接
                printf("new connect!\n");
				while ((connfd = accept(listenfd,(struct sockaddr *) &cli_addr, &cli_len)) > 0) {
                    printf("while circle\n");
                    setNonBlocking(connfd);
                    printf("after setnonblocking");
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = connfd;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd,&ev) == -1) {
                        perror("epoll_ctl: add");
                        exit(EXIT_FAILURE);
                    }
                }
                 if (connfd == -1) {
                    if (errno != EAGAIN && errno != ECONNABORTED
                            && errno != EPROTO && errno != EINTR)
                        perror("accept");
                }
                continue;
                printf("there?\n");
			}
			/* 监听到读事件 */
			if (events[i].events & EPOLLIN) {
                printf("epollin!\n");
                //--------------------------------------------------------
                n = 0;
                int   flag =1,sockgo;
                struct sockaddr_in servaddr;
                while ((nread = read(fd, buf + n, MAXBUF-1)) > 0) {
                    printf("%s--\n",buf);
                    n += nread;
                }
                if (nread == -1 && errno != EAGAIN) {
                    perror("read error");
                }

                 /*
                     转发转发*/


                char ip[32];
                if(flag){
                    DNtoIP(buf,ip);
                }
                printf("IP is:%s\n",ip);

                socklen_t len;
                sockgo = socket(AF_INET,SOCK_STREAM,0);
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(80);
                inet_pton(AF_INET,ip,&servaddr.sin_addr);

                connect(sockgo,(struct sockaddr *)&servaddr,sizeof(servaddr));
                printf("read from web\n");
                write(sockgo,buf,strlen(buf));
                fprintf(getinfo,"%s\n",ip);
                fprintf(getinfo,"%s\n",buf);



                read(sockgo,strbuf,4095);
                fprintf(getAns,"%s\n",ip);
                fprintf(getAns,"%s\n",strbuf);

                ev.data.fd = fd;
                ev.events = events[i].events | EPOLLOUT;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                    perror("epoll_ctl: mod");
                }
            }
            if (events[i].events & EPOLLOUT) {
                printf("epollout!\n");
                //-------------------------------------------------------
                printf("%s\n",strbuf);
                int nwrite, data_size = strlen(strbuf);
                n = data_size;
                while (n > 0) {
                    printf("send to brower cirlce\n");
                    nwrite = write(fd, strbuf +max(0, data_size - n), n);
                    printf("send done\n");
                    if (nwrite < n) {
                        if (nwrite == -1 && errno != EAGAIN) {
                            perror("write error");
                        }
                        break;
                    }
                    n -= nwrite;
                }
                //-------------------------------------------------------
                close(fd);
            }
		}//nfds for
	}//for

	/* 关闭套接字 */
	close(listenfd);
	return 0;
}

/* usersockfd向remtsockfd传送数据，http请求 */

static void* TransWorker(void* arg){
    printf("enter TransWorker \n");
    mTrans_t *pstru;
	pstru = (mTrans_t *)arg;
	char buf[MAXBUF];
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
	printf("HOST:%s\n",domainName);
	if((hptr = gethostbyname(domainName)) == NULL){
		perror("gethostbyname");
		return 0;
	}
	strcpy(ipstr,inet_ntoa(*((struct in_addr *)hptr->h_addr)));
	//printf("IP :%s\n",inet_ntoa(*((struct in_addr *)hptr->h_addr)));
	printf("IP :%s\n",ipstr);
	free(domainName);
	return ipstr;
}

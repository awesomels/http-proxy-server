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
#define MAXBUF 1024
#define BIGBUF 409600
#define N 100
#define MAXEVENTS 128

int main(){

    struct epoll_event ev, events[20];
    int epfd,nfds, i, connfd, n,sockfd,fd;
    struct sockaddr_in local_addr;  //本地监听地址
    struct sockaddr_in cli_addr;    //客户端地址结构
    socklen_t cli_len = sizeof(cli_addr);
    int listenfd;
    char strbuf[BIGBUF];
	char buf[MAXBUF];
	int on=1;
    int index ;


    printf("**working**\n");


	if( (listenfd=socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("listenfd error");
		exit(1);
	}
	/* 设置本地地址结构体 */
	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(Port);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	if( setNonBlocking(listenfd) < 0)//设置套接字为非阻塞
	{
        perror("setNonBlocking");
        exit(1);
	}
    if( (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0 )//设置端口复用
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
	if( bind(listenfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) != 0 )
	{
		perror("bind error");
		close(listenfd);
		exit(1);
	}
	if( listen(listenfd, SOMAXCONN) != 0 )
	{
		perror("listen error");
		close(listenfd);
		exit(1);
	}

	/*
	 *使用epoll
	 */
	if( (epfd=epoll_create1(0)) == -1 )  //除了参数size被忽略外，此函数和epoll_create完全相同
	{
		perror("epoll_create");
		abort();
	}
	ev.data.fd = listenfd;    //设置与要处理的事件相关的文件描述符
	ev.events = EPOLLIN|EPOLLET;//设置要处理的事件类型
	if( epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1 )
	{
		perror("epoll_ctl");
		abort();
	}
	/* the event loop */
	printf("epoll loop\n");
	for ( index =0 ; ; ++ index)
	{
        if ( (nfds=epoll_wait(epfd, events, MAXEVENTS, -1)) == -1)  //epoll_wait
        {
            perror("epoll_pwait");
            exit(EXIT_FAILURE);
        }
        printf("epoll loop count=%d\tevent num=%d\n",index,nfds);
		for(i = 0; i < nfds; ++i)
		{
            printf("inside for\n");
            fd = events[i].data.fd;
			if(fd == listenfd)  //有新的连接
			{
                printf("-----------------new connect!\n");
				while ( (connfd=accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len)) > 0 )
				{
                    setNonBlocking(connfd);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = connfd;
                    if ( epoll_ctl(epfd, EPOLL_CTL_ADD, connfd,&ev) == -1 )
                    {
                        perror("epoll_ctl: add");
                        exit(EXIT_FAILURE);
                    }
                }
                if (connfd == -1)
                {
                    printf("accept return -1\n");
                    if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
                        perror("accept");
                }
                continue;
			}
			else if (events[i].events & EPOLLIN)  //监听到读事件
            {
                printf("-----------------epollin!\n");
                //--------------------------------------------------------

                struct sockaddr_in servaddr;
                socklen_t len=sizeof(servaddr);
                char ip[32];
                int sockgo,nread;
                FILE *cachein;

                while ((nread = read(fd, buf, MAXBUF-1)) > 0) {
                    printf("HTTP请求打印\n%sHTTP 请求结束\nnread=%d\n",buf,nread);
                    cachein=fopen("cachein","w");
                    fputs(buf, cachein);  //向cachein写入
                    fclose(cachein);
                }
                if (nread == -1 && errno != EAGAIN) {
                    perror("read error");
                }
                /*转发转发*/
                DNtoIP(buf,ip);
                printf("IP is:%s\n",ip);
                if( (sockgo=socket(AF_INET, SOCK_STREAM, 0)) < 0 )
                {
                    perror("sockgofd error");
                    exit(1);
                }
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(80);
                inet_pton(AF_INET,ip,&servaddr.sin_addr);
                if( (connect(sockgo,(struct sockaddr *)&servaddr,sizeof(servaddr))) < 0 )
                {
                    printf("connect failed!\n");
                }
                printf("read from web\n");

                write(sockgo,buf,strlen(buf));

                cachein=fopen("cacheout","w");
                while( (nread=read(sockgo, strbuf, BIGBUF-1)) > 0)
                {
                    printf("HTTP响应打印\n%s\nHTTP响应结束\nnread=%d\n",strbuf,nread);
                    fputs(strbuf, cachein);//向cacheout写入
                    bzero(strbuf, BIGBUF);
                }
                fclose(cachein);

                ev.data.fd = fd;
                ev.events = events[i].events | EPOLLOUT;
                if ( epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1 )
                {
                    perror("epoll_ctl: mod");
                }
            }
            else if (events[i].events & EPOLLOUT)  //监听到写事件
            {
                printf("-----------------epollout!\n");
                //-------------------------------------------------------

                //char buf2[256];
                //sprintf(buf2, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nHello World", 11);
                //write(fd, buf2, 255);

                FILE *cacheout;
                cacheout=fopen("cacheout","r");
                while ( !feof(cacheout) )
                {
                    fread(strbuf, BIGBUF-1, N, cacheout);//从cacheout读出
                    //fscanf(cacheout, "%s", strbuf);
                    write(fd, strbuf, BIGBUF-1);
                    printf("从文件读%s\n",strbuf);
                }
                fclose(cacheout);

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

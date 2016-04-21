#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc,char *argv[])
{
	unsigned short port = 8000;//本地端口
	if(argc > 1)
	{
		port = atoi(argv[1]);
	}
	//1.创建通信端点：套接字
	int sockRecfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockRecfd < 0)
	{
		perror("socket");
		exit(-1);
	}
	//设置本地地址结构体
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));//清空，保证最后8字节为0
	my_addr.sin_family = AF_INET;//ipv4
	my_addr.sin_port = htons(port);//端口.htons将主机的无符号短整型数转换成网络字节序
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//ip,INADDR_ANY为通配地址其值为0.将主机的无符号长整型数转换为网络字节序

	//2.绑定：将本地ip、端口与套接字socket相关联起来
	int err_log = bind(sockRecfd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if(err_log != 0)
	{
		perror("binding");
		close(sockRecfd);
		exit(-1);
	}

	//3.监听：监听套接字改为被动，创建连接队列
	err_log = listen(sockRecfd,10);
	if(err_log != 0)
	{
		perror("listen");
		close(sockRecfd);
		exit(-1);
	}

	printf("listen client @port = %d...\n",port);

	while(1)
	{
		struct sockaddr_in client_addr;
		char cli_ip[INET_ADDRSTRLEN] = "";
		socklen_t cliaddr_len = sizeof(client_addr);

		int connRecfd = 0;

		//4.从完成连接队列中提取客户端连接
		connRecfd = accept(sockRecfd, (struct sockaddr*)&client_addr, &cliaddr_len);
		if(connRecfd < 0)
		{
			perror("accept");
			continue;
		}
		
		inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);
		printf("----------------------------------------------------\n");
		printf("client ip = %s,port = %d\n",cli_ip,ntohs(client_addr.sin_port));

		char recv_buf[512] = "";
		while(recv(connRecfd, recv_buf, sizeof(recv_buf), 0) > 0)//接收数据
		{
			printf("\nrecv data:\n");
			printf("%s\n",recv_buf);
		}

		close(connRecfd);//关闭已连接套接字
		printf("client closed!\n");
	}

	close(sockRecfd);//关闭监听套接字

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 4096

typedef struct Hostname{
	int length;
	char *src;
}Hostname,*phost;

struct hostent *hptr;

/*----------------getHost()---------------------------
 *
 *获取http请求的主机信息，存储在Hostname结构体中
 *Hostname结构体存储主机名的起始地址和主机名长度
 *
 **/
Hostname getHost(const char* m_string)
{
	char *substr = "Host: ";
	Hostname m_stu;
	m_stu.src = strstr(m_string, substr) + 6;
	char *p_char = m_stu.src;
	m_stu.length = 0;
	while(*p_char != '\r'){
		m_stu.length++;
		p_char++;
	}
	return m_stu;
}//--------------------getHost()----------------------



/*--------------------main()----------------------------
 *
 *主函数
 *
 */
int main(int argc,char *argv[])
{
	//hostname结构体
	Hostname hostname;
	bzero(&hostname, sizeof(hostname));
	
	unsigned short port = 8000;//本地端口
	if(argc > 1)
	{
		port = atoi(argv[1]);
	}
	//1.创建通信端点：套接字
	int sockRecfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockRecfd < 0)
	{
		perror("recvsocket");
		exit(-1);
	}
	/*添加发送socket*/
	int sockSenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockSenfd < 0)
	{
		perror("sendsocket");
		exit(-1);
	}

	//设置本地地址结构体 my_addr
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));//清空，保证最后8字节为0
	my_addr.sin_family = AF_INET;//ipv4
	my_addr.sin_port = htons(port);//端口.htons将主机的无符号短整型数转换成网络字节序
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//ip,INADDR_ANY为通配地址其值为0.将主机的无符号长整型数转换为网络字节序
	/*
	 *添加目的地址结构体 da_addr
	 *目的IP地址暂时未知
	 *da_addr.sin_addr.s_addr
	 */
	struct sockaddr_in da_addr;
	bzero(&da_addr, sizeof(da_addr));
	da_addr.sin_family = AF_INET;
	da_addr.sin_port = htons(80);


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

		
		char recv_buf[BUF_SIZE] = "";//缓冲buffer
		char ipstr[32];//目的IP地址
		while(recv(connRecfd, recv_buf, sizeof(recv_buf), 0) > 0)//接收数据
		{
			hostname = getHost(recv_buf);
			char *domainName = (char*)malloc(hostname.length);//malloc domainName
			strncpy(domainName, hostname.src, hostname.length);
			printf(" %s\n", domainName);
			if((hptr = gethostbyname(domainName)) == NULL){
				printf(" gethostbyname error for host:%s\n", domainName);
				printf(" size of domainName:%ld\n", strlen(domainName));
				herror("gethostbyname");
				char mm[] = "www.baidu.com";
				printf(" size of mm:%ld\n", strlen(mm));
				return 0;
			}			
			switch(hptr->h_addrtype){
				case AF_INET:
				case AF_INET6:
					printf(" first address: %s\n", inet_ntop(hptr->h_addrtype, \
											hptr->h_addr, ipstr, sizeof(ipstr)));
				break;
				default:
					printf(" unknown address type\n");
				break;
			}//switch
			free(domainName);//free domainName
			printf("\nrecv data:\n");
			printf("%s\n", recv_buf);
		}//while(recv)

		close(connRecfd);//关闭已连接套接字
		printf("client closed!\n");
	}//while(1),主循环

	close(sockRecfd);//关闭监听套接字

	return 0;
}//---------------------------main()--------------------------

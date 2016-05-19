#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(){

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
	bzero(&user_addr, sizeof(useraddr));
	user_addr.sin_family = AF_INET;
	user_addr.sin_port = htons(Port);
	user_addr.sin_addr.s_addr = htol(INADDR_ANY);
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

	/*
	 *
	 */

	return 0;
}

static void* UtoR(){
	
}

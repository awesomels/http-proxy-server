#ifndef __MAIN_H__
#define __MAIN_H__

#define MAXEVENTS 64
#define BUF_SIZE 4096

/* UtoR线程参数 */
typedef struct mTrans{
	int mUsocket;
	int mRsocket;
}mTrans_t;



/*
 *@brief:  域名转IP, 提供给发送socket
 *@param:  mString, 供提取域名的字符串,未筛选
 *@return: ipstr[32], 返回的ip地址,32位字符数组
 */
char *DNtoIP(mString);

/*
 *@brief:  设置socket为非阻塞模式
 *@param:  mSocket套接字
 *@return: 0:正常
 */
void setNonBlocking(int mSocket);

/*
 *@brief:  从usersockfd向remtsockfd传送http请求,接收处理并最后返回给usersockfd
 *@param:  
 *@return: void
 */
static void* TransWorker(void* arg);


#endif

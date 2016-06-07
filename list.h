#ifndef __LIST_H__
#define __LIST_H__

typedef struct LNode{
	struct LNode* next;  //指针域
	char  cli_ip[32];    //数据域
	char  tgt_ip[32];
	int   cli_ip;
	char* tgt_url;
}LNode;

typedef struct LList{
	
}LList;

#endif

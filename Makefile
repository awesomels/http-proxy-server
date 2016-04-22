#生成proxy
proxy:proxy.o
	gcc -o proxy proxy.o
#生成proxy.o的规则
proxy.o:proxy.c
	gcc -c -o proxy.o proxy.c 
#清理的规则
clean:
	rm -f proxy proxy.o

#生成proxy
proxy:proxy.o thread.o
	gcc -o proxy proxy.o thread.o -lpthread
#生成proxy.o的规则
proxy.o:proxy_main.c thread_pool.c
	gcc -c -o proxy.o proxy_main.c
	gcc -c -o thread.o thread_pool.c -lpthread
#清理的规则
clean:
	rm -f proxy proxy.o thread.o 

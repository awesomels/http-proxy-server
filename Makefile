#生成proxy
proxy:proxy.o thread.o
	gcc -o proxy proxy.o filter.o thread.o -lpthread -lmysqlclient
#生成proxy.o的规则
proxy.o:proxy_main.c thread_pool.c
	gcc -c -o proxy.o proxy_main.c
	gcc -c -o thread.o thread_pool.c -lpthread
	gcc -c -o filter.o filter.c -lmysqlclient
#清理的规则
clean:
	rm -f proxy proxy.o thread.o filter.o

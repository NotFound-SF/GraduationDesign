#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <fcntl.h>


#define  MyPort        2048
#define  BUF_SIZE      1024
#define  EPOLL_SIZE    128                     // 一次性返回的请求列表
#define  CLIENT_MAX    2048                    // 最大客户连接数,实际比该值小4
#define  THREAD_COUNT  32                      // 默认创建的线程数

#define  ERR_EXIT(m)   do {perror(m); exit(EXIT_FAILURE);} while(0)


struct task_queue{
	int               cfd;                     // 文件描述符号
};



int set_fl(int fd, int flags);
int clr_fl(int fd, int flags);
int initServer(int type, const struct sockaddr *saddr, 
		       socklen_t alen, int qlen);










	

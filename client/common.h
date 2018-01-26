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
#define  TIMECOUNT     5                       // 超时次数
#define  TIMECYCLE     3                       // 超时检测周期3s
#define  GRANULARITY   10                      // 加锁粒度
#define  BUF_SIZE      1024
#define  INITDID       0xFFFFFFFF
#define  EPOLL_SIZE    128                     // 一次性返回的请求列表
#define  CLIENT_MAX    2048                    // 最大客户连接数,实际比该值小4
#define  THREAD_COUNT  32                      // 默认创建的线程数

#define  ERR_EXIT(m)   do {perror(m); exit(EXIT_FAILURE);} while(0)

typedef unsigned int   uint32_t;

struct task_queue{
	int               cfd;                     // 文件描述符号
};

struct fdinfo {
	uint32_t  DID;                            // 设备或者用户ID
	int       timeout;                        // 相应超时
};


struct datformat {
	uint32_t  selfID;
	uint32_t  targetID;
	char      data[4];                     
};


int set_fl(int fd, int flags);
int clr_fl(int fd, int flags);
int initServer(int type, const struct sockaddr *saddr, 
		       socklen_t alen, int qlen);










	

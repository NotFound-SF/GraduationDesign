
#include "common.h"
#include <pthread.h>
#include <queue>
#include <sys/resource.h>

using namespace std;


struct workinfo{
	pthread_cond_t    qready;
	pthread_mutex_t   qlock;
	queue<task_queue> workq;                  // 待处理工作队列
};

static unsigned  int     num = 0;
static int               epollfd;
static pthread_mutex_t   elock = PTHREAD_MUTEX_INITIALIZER;
static struct workinfo   *worklist;           // 线程工作列表 

void *handle_thread(void *arg);


int main(int argc, char *argv[]) 
{
	int        listenfd;
	int        clientfd;
	int        thread_count;                  // 要创建的线程数
	int        nready; 
	int        which;
	int        index;
	socklen_t  clilen;
	pthread_t  tid;
	struct     sockaddr_in  servaddr;
	struct     sockaddr_in  cliaddr;
	struct     epoll_event  event;
	struct     epoll_event  *epevents;
	struct     task_queue   newwork;
	struct     rlimit       flimit;

	if (1 == argc) 
		thread_count = THREAD_COUNT;    
	else if(2 == argc) {
		thread_count = atoi(argv[1]);
	} else {
		fprintf(stderr, "argv error");
		exit(1);
	}

	// 确保不会产生僵尸进程
	if (SIG_ERR == signal(SIGCHLD, SIG_IGN))
		ERR_EXIT("signal");

	// 修改文件限制
	flimit.rlim_cur = CLIENT_MAX;
	flimit.rlim_max = CLIENT_MAX;
	if (setrlimit(RLIMIT_NOFILE, &flimit) < 0)
	{
		perror("setrlimit");
		printf("MAXFILE: %ld\n", sysconf(_SC_OPEN_MAX));
	}

	// Create addr
	bzero(&servaddr, sizeof(&servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(MyPort);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Inite server
	listenfd = initServer(SOCK_STREAM, (struct sockaddr*)&servaddr,
						  sizeof(servaddr), SOMAXCONN);
	if (listenfd < 0) 
		ERR_EXIT("initSever");
	set_fl(listenfd, O_NONBLOCK);                                 // 设置为非阻塞

	// 创建epoll实例
	if ((epollfd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		ERR_EXIT("epoll_create");
	
	// 初始化event关联listenfd的输入事件,并且条件到epollfd实例
	event.events = EPOLLIN;
	event.data.fd = listenfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) < 0)	
		ERR_EXIT("epoll_ctl");
	
	// 存放被更新的集合
	epevents = (struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
	if (NULL == epevents) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	// 创建处理线程与处理空间
	worklist = (struct workinfo*)malloc(thread_count*sizeof(struct workinfo));
	if (NULL == worklist) {
		fprintf(stderr, "malloc error for worklis\n");
		exit(1);
	}
	for (index = 0; index < thread_count; index++) {
		pthread_mutex_init(&worklist[index].qlock, NULL);
		pthread_cond_init(&worklist[index].qready, NULL);
		new (&worklist[index].workq) queue<task_queue>();            // 初始化队列
		pthread_create(&tid, NULL, &handle_thread, (void*)index);
	}


	// 主逻辑
	while (1) {
		nready = epoll_wait(epollfd, epevents, EPOLL_SIZE, -1);      // wait long				
		if (nready < 0)
			ERR_EXIT("epoll_wait");                                  // 失败直接退出
		else if(0 == nready)
			continue;

		for (index = 0; index < nready; index++) {
			if (epevents[index].data.fd == listenfd) {               // 表示有新的连接上来
				clilen = sizeof(cliaddr);
				clientfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
				set_fl(clientfd, O_NONBLOCK);                        // 设置为非阻塞
				event.events = EPOLLIN|EPOLLET;                      // 边缘触发方式
				event.data.fd = clientfd;

				pthread_mutex_lock(&elock);
				epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event);
				num++;
				pthread_mutex_unlock(&elock);

				printf("From:%s Port:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
				printf("Client num:%u\n", num);
			} else {                                                // 表示客户端有数据交付
				// 将新的任务加入待处理队列
				which = epevents[index].data.fd % thread_count;     // 选择队列
				newwork.cfd = epevents[index].data.fd;              // 初始化要加入队列的数据
				pthread_mutex_lock(&worklist[which].qlock);
				if (worklist[which].workq.empty()) {
					worklist[which].workq.push(newwork);
					pthread_cond_signal(&worklist[which].qready);   //线程需要唤醒,可能多余		
				} else {
					worklist[which].workq.push(newwork);
				}
				pthread_mutex_unlock(&worklist[which].qlock);
			}
		}
	}

	close(listenfd);
	close(epollfd);
	free(epevents);

	printf("Done\n");

	exit(0);
}



// 处理线程
void *handle_thread(void *arg)
{
	int               mytid;
	int               str_len;
	char              buf[BUF_SIZE];
	struct task_queue my_work;

	mytid = (int) arg;
	
	while(1) {
		pthread_mutex_lock(&worklist[mytid].qlock);                                   // 锁定就绪队列		
		while(worklist[mytid].workq.empty())        
			pthread_cond_wait(&worklist[mytid].qready, &worklist[mytid].qlock);       // 等待条件成立
		my_work = worklist[mytid].workq.front();                                      // 从就绪队列取出一个任务
		worklist[mytid].workq.pop();                                                  // 从队列删除一个取出的任务
		pthread_mutex_unlock(&worklist[mytid].qlock);                                 // 锁定就绪队列		

		// 处理主任务发来的任务
		while (1) {
			str_len = recv(my_work.cfd, buf, BUF_SIZE, 0);
			if (0 == str_len) {                                                       // 表示客户端断啦
				pthread_mutex_lock(&elock);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, my_work.cfd, NULL);
				num--;
				pthread_mutex_unlock(&elock);
				close(my_work.cfd);
				printf("Client num:%u\n", num);
				break;
			} else if (str_len < 0){                        
				if (EAGAIN == errno)                                                  // 表示缓冲区无数据可读
					break;
				else
					break;                                                            // 表示错误文件号
			} else {
				send(my_work.cfd, buf, str_len, 0);
			}
     	}
	}

	return 0;
}







#include "common.h"
#include <pthread.h>
#include <queue>
#include <sys/resource.h>
#include <ext/hash_map>

using namespace __gnu_cxx;
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

hash_map<int, fdinfo>    fdinfomap;               
hash_map<uint32_t, int>  keyfdmap;



void *time_thread(void *arg);
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
	struct     fdinfo       newfd;

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


	// 创建超时检测线程
	pthread_create(&tid, NULL, &time_thread, NULL);                  // 该线程会释放资源


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
				newfd.DID = INITDID;                                 // 初始化为无效
				newfd.timeout = TIMECOUNT;                           // 初始化倒计时
				pthread_mutex_lock(&elock);
				epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event);
				fdinfomap[clientfd] = newfd;                         // 将新链接的客户插入到fd-key表
				num++;
				pthread_mutex_unlock(&elock);

				printf("From:%s Port:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
				printf("Client num:%u\n", num);
			} else {                                                // 表示客户端有数据交付
				// 将新的任务加入待处理队列
				which = epevents[index].data.fd % thread_count;     // 选择队列
				newwork.cfd = epevents[index].data.fd;              // 初始化要加入队列的数据
				pthread_mutex_lock(&worklist[which].qlock);
				if (worklist[which].workq.empty()) {                // 待处理任务队列为空发信号
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
	int               tagfd;
	int               str_len;
	struct datformat  buf;
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
			str_len = recv(my_work.cfd, &buf, sizeof(struct datformat), 0);
			if (0 == str_len) {                                                       // 表示客户端断啦
				pthread_mutex_unlock(&elock);
				if (0 == fdinfomap[my_work.cfd].timeout) {                            // 表示已经被超时删除
					pthread_mutex_unlock(&elock);
					break;                         
				}
				// 标记为删除 0表示将要删除
				fdinfomap[my_work.cfd].DID = 0;                                       // 0标记该节点被删除
				pthread_mutex_unlock(&elock);
				break;
			} else if (str_len < 0){                        
				if (EAGAIN == errno)                                                  // 表示缓冲区无数据可读
					break;
				else
					break;                                                            // 表示错误文件号
			} else {                                                                  // 表示有真实数据
				// 刷新超时参数,取出通信目标fd
				pthread_mutex_lock(&elock);
				if (0 == fdinfomap[my_work.cfd].timeout) {                            // 表示已经被超时删除
					pthread_mutex_unlock(&elock);
					break;                         
				}

				fdinfomap[my_work.cfd].timeout = TIMECOUNT;
				if (INITDID == fdinfomap[my_work.cfd].DID) {                           // 该链接第一次真实通信添加key->dat
					fdinfomap[my_work.cfd].DID = buf.selfID;	                       // 在main线程已经插入
					keyfdmap[buf.selfID] = my_work.cfd;                                // 将其添加到key-fd表
				}

				// 通过目标ID找到指定指定文件描述符,0表示目标设备未上线 
				tagfd = keyfdmap[buf.targetID];                                     
				pthread_mutex_unlock(&elock);

				if (0 != buf.targetID) {
					if (0 != tagfd) {
						send(tagfd, &buf, sizeof(datformat), 0);                    // 去掉了目标和头信息需要修改
					}else {                                                                // 表示设备断线
						//send(my_work.cfd, "outlin", sizeof(datformat), 0);               // 提示自己
					}
				} else {                                                                   // 脉搏信息不处理
					printf("ID: %d\talive\n", buf.selfID);
				}
			}
     	}
	}

	return 0;
}



// 该线程定时递减文件描述符中的标志，以判断设备是否掉线
// 以及释放只连接占用资源的恶意用户资源
void *time_thread(void *arg)
{
	int            count;
	uint32_t       tmpID;
	int            tmpfd;
  	hash_map<int, fdinfo>::iterator itr;                                               // 迭代容器
	
	arg = arg;

	while (1) {
		// 遍历所有连接的超时时间
		pthread_mutex_lock(&elock);
		for(itr = fdinfomap.begin(); itr != fdinfomap.end(); count++) {
			
			tmpID = itr->second.DID;                                                   // 取得设备ID
			itr -> second.timeout--;                                                   // 超时递减	

			if (itr->second.timeout <= 0 || 0 == tmpID) {                              // 表示超时或者客户放弃连接            
				tmpfd = itr->first;                                                    // 取得文件描述符号
				epoll_ctl(epollfd, EPOLL_CTL_DEL, tmpfd, NULL);                        // 从epoll中移除
				//删除队列中的标记
				keyfdmap.erase(tmpID);                                                 // 通过key删除
				fdinfomap.erase(itr++);                                               
				num--;
				close(tmpfd);
				printf("Client num:%u\n", num);
			} else {
				itr++;
			}
			// 减小加锁粒度，不让处理线程长时间挂起
			if (count>GRANULARITY){
				count = 0;
				pthread_mutex_unlock(&elock);                                          // 释放锁
				usleep(10);                                                            // 切换一次线程，让处理线程执行
				pthread_mutex_lock(&elock);                                            // 重新获取锁
			}
		}
		pthread_mutex_unlock(&elock);                                                  // 遍历完一次解锁
		
		sleep(TIMECYCLE);                                                              // 睡眠该线程
	}

	return 0;
}




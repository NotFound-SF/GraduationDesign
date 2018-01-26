#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include "common.h"
#include <pthread.h>


#define     MAXSLEEP      128      // about 2 mins

static int connect_retry(int domain, int type, int protocol,
         				 const struct sockaddr *addr, socklen_t alen);


int                sfd;
struct datformat   data;
struct datformat   live;
pthread_mutex_t    lock = PTHREAD_MUTEX_INITIALIZER;


void *alive(void *arg) 
{
	while (1) {
		pthread_mutex_lock(&lock);
		send(sfd, &live, sizeof(struct datformat), 0);
		pthread_mutex_unlock(&lock);

		sleep(2);                // 2s抖动一次
	}
	return 0;
}


int main(int argc, char *argv[])
{
	int                stdinfd;
	int                maxfd;
	int                nselec;
	fd_set             rfd_set;
	char               buf[BUF_SIZE];
	int                nWrite;
	int                nRead;
	struct sockaddr_in addr;
	pthread_t          tid;

	if (4 != argc) {
		fprintf(stderr, "argc error\n");
		exit(1);
	}


	data.selfID = atoi(argv[2]);    // 设置设备ID
	live.selfID = atoi(argv[2]);
	data.targetID = atoi(argv[3]);
	live.targetID = 0;

	signal(SIGPIPE, SIG_IGN);

	// 1. Craet addr
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port  = htons(MyPort);
	if (inet_pton(addr.sin_family, argv[1], &addr.sin_addr) < 0) {
		fprintf(stderr, "inet_pton faild\n");
		exit(1);
	}

	// 2.Creat socket and connecting 
	sfd = connect_retry(addr.sin_family, SOCK_STREAM, 0, 
						(struct sockaddr *)&addr, sizeof(addr));
	if (sfd < 0) {
		fprintf(stderr, "connect_retry faild\n");
		exit(1);
	}

	pthread_create(&tid, NULL, alive, NULL);

	// 3. Send data
    stdinfd = fileno(stdin);
	fflush(stdin);
	maxfd = stdinfd > sfd ? stdinfd: sfd;
	while (1) {
		FD_SET(stdinfd, &rfd_set);
		FD_SET(sfd, &rfd_set);
		nselec = select(maxfd+1, &rfd_set, NULL, NULL, NULL);	
		
		if (nselec > 0) {
			if (FD_ISSET(sfd, &rfd_set)) {
				if((nRead = recv(sfd, buf, 12, 0)) < 1)
					break;
				else {
					buf[12] = 0;
					printf("From: %d : %s\n",((int*)buf)[0], buf+8);
				}
			}  

			if(FD_ISSET(stdinfd, &rfd_set))	{
				if (NULL == fgets(buf, 5, stdin))
					break;
				else {
					data.data[0] = buf[0];
					data.data[1] = buf[1];
					data.data[2] = buf[2];
					data.data[3] = buf[3];
					pthread_mutex_lock(&lock);
					send(sfd, &data, sizeof(struct datformat), 0);
					pthread_mutex_unlock(&lock);
				}
				fflush(stdin);
			}
			
		} else {
			fprintf(stderr, "select error\n");
		}
	}


	// 5. Close sfd
	close(sfd);

	printf("Done\n");

	exit(0);
}


static int connect_retry(int domain, int type, int protocol,
				         const struct sockaddr *addr, socklen_t alen)
{
	int numsec, sfd;

	// if the connection fails in some systems, the sfd will be undefined
	for (numsec = 1; numsec < MAXSLEEP; numsec <<= 1) {
		
		// 1.Creat socket
		if ((sfd = socket(domain, type, protocol)) < 0) {
			fprintf(stderr, "socket error\n");
			exit(1);
		}

		// 2.Try to connect address
		if (0 == connect(sfd, addr, alen)) {
			return (sfd);
		} 
		close(sfd);                                           // connection failed,close socket

		// 3.Delay before trying agin
		if (numsec < MAXSLEEP/2) {
			sleep(numsec);
		}
	}

	return (-1);                                              // failed
}




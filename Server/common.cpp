
#include "common.h"




// 初始化服务器，创建socket 绑定端口 listen, 成功返回fd失败返回-1
// type为协议类型;saddr为地址;alen地址长度;qlen监听队列长度
int initServer(int type, const struct sockaddr *saddr, 
		       socklen_t alen, int qlen)
{
	int     sfd, err;                      // socket file description
	int     reuse = 1;                     // turn on the option     	

	// 1.Creat socket 
	if ((sfd = socket(saddr->sa_family, type, 0)) < 0) {
		return (-1);
	}

	// 2.Reuse the saddr
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		goto errout;
	
	// 3.Bind socket to saddr
	if (bind(sfd, saddr, alen) < 0)
		goto errout;
     
	// 4.Listen the socket
	if (SOCK_STREAM == type || SOCK_SEQPACKET == type)
		if (listen(sfd, qlen) < 0)
			goto errout;
	
	return (sfd);

errout:
	err  = errno;
	close(sfd);
	errno = err;
	return (-1);
}


// 设置文件标志成功返回0失败返回-1
int set_fl(int fd, int flags)
{
	int  val;

	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		return -1;

	val |= flags;

	if (fcntl(fd, F_SETFL, val) < 0)
		return -1;

	return 0;
}



// 清除文件标志成功返回0失败返回-1
int clr_fl(int fd, int flags)
{
	int  val;

	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		return -1;

	val &= ~flags;

	if (fcntl(fd, F_SETFL, val) < 0)
		return -1;

	return 0;
}



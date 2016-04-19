#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>

#define IPADDRESS   "127.0.0.1"
#define PORT        8787
#define MAXLINE     1024
#define LISTENQ     5
#define OPEN_MAX    1000
#define INFTIM      -1



 //函数声明
 //创建套接字并进行绑定
 static int socket_bind(const char* ip,int port);
 //IO多路复用poll
 static void do_poll(int listenfd);
 //处理多个连接
 static void handle_connection(struct pollfd *connfds,int num);




int main(int argc,char* argv[])
{
	int listenfd,connfd,sockfd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;

	listenfd=socket_bind(IPADDRESS,PORT);
	listen(listenfd,LISTENQ);
	do_poll(listenfd);

	return 0;
}


static int socket_bind(const char* ip,int port)
{
	int listenfd;
	struct sockaddr_in servaddr;
	listenfd = socket(AF_INET,SOCK_STREAM,0);

	if(listenfd==-1)
	{
		perror("listen");
		exit(1);
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	inet_pton(AF_INET,ip,&servaddr.sin_addr);
	servaddr.sin_port=htons(port);

	if( bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}

	return listenfd;
}


static void do_poll(int listenfd)
{
	int connfd,sockfd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	struct pollfd clientfds[OPEN_MAX];

	int maxi;
	int i;
	int nready;
	//添加监听描述符
	clientfds[0].fd=listenfd;
	clientfds[0].events = POLLIN;

	//初始化客户连接描述符
	for(i=1;i<OPEN_MAX;++i)
		clientfds[i].fd=-1;
	maxi=0;

	for(;;)
	{
		//获取可用文件描述符的个数
		nready = poll(clientfds,maxi+1,INFTIM);/*timeout指定为负数值表示无限超时，使poll()一直挂起直到一个指定事件发生*/
		if(nready==-1)
		{
			perror("poll");
			exit(1);
		}

		//测试监听描述符是否准备好
		if(clientfds[0].revents & POLLIN)
		{
			cliaddrlen = sizeof(cliaddr);

			//接受新的连接
			if( (connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen))==-1)
			{
				if(errno==EINTR)
					continue;
				else
				{
					perror("accept");
				 	exit(3);
				} 
			}
			 fprintf(stdout,"accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);


			 //将新的文件描述符添加到数组中
			 for(i=1;i<OPEN_MAX;++i)
			 {
				 if(clientfds[i].fd < 0) 
				 {
					 clientfds[i].fd=connfd;
					 break;
				 }
			 }

			 if(i==OPEN_MAX)
			 {
				 fprintf(stderr,"too many clients\n");
				 exit(1);
			 }

			 //将新的描述符添加文件描述符集合中
			 clientfds[i].events = POLLIN;

			 maxi=(i>maxi ? i: maxi);
			 if(--nready <= 0)
			 {
				 continue;
			 }

		}

		//处理客户链接
		handle_connection(clientfds,maxi);
	}
}


static void handle_connection(struct pollfd * connfds,int num)
{
	int i,n;
	char buf[MAXLINE];
	memset(buf,0,MAXLINE);

	for(i=1;i<=num;++i)
	{
		if(connfds[i].fd<0)
			continue;

		//测试客户描述符是否准备好
		if(connfds[i].revents & POLLIN)
		{
			//接受客户发送的信息
			n=read(connfds[i].fd,buf,MAXLINE);
			if(n==0)
			{
				close(connfds[i].fd);
				connfds[i].fd=-1;
				continue;
			}
		write(STDOUT_FILENO,buf,n);
		write(connfds[i].fd,buf,n);
		}
	}
}

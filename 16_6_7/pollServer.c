#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<poll.h>


#define BACKLOG 5
#define SIZE 64

static int start(char* ip,short port)
{
	int sock=socket(AF_INET,SOCK_STREAM,0);  
	if(sock<0)
	{
		perror("socket");
		exit(2);
	}

	//bind
	struct sockaddr_in local;

	local.sin_family=AF_INET;
	local.sin_port=htons(port);
	inet_pton(AF_INET,ip,&local.sin_addr);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		perror("bind");
		exit(3);
	}

	if( listen(sock,BACKLOG) < 0 )
	{
		perror("listen");
		exit(4);
	}

	return sock;
}

int main(int argc,char* argv[])
{	
	if(argc!=3)
	{
		printf("Usage %s [ip] [port]\n",argv[0]);
		exit(1);
	}

	int _port =atoi(argv[2]);
	char* _ip=argv[1];

	int listen_sock=start(_ip,_port);

	struct pollfd polls[SIZE];
	int index = 0; //effective fdnum
	int timeout = 5000;
	int i=0;
	polls[0].fd=listen_sock;
	polls[0].events=POLLIN;
	polls[0].revents=0;
	int max_num=1;
	
	for(i=1;i<SIZE;++i)
		polls[i].fd=-1;

	char buf[1024];
	ssize_t sz=0;

	struct sockaddr_in remote;
	socklen_t len=sizeof(remote);

	while(1)
	{
		//timeout=5000;

		switch(poll(polls,max_num,timeout))
		{
			case 0:
				printf("time out...\n");
			break;

			case -1:
				perror("poll");
			break;

			default:
				{
					for(i=0;i<max_num;++i)
					{
						if(polls[i].fd==listen_sock && (polls[i].revents \
								& POLLIN))
						{
							printf("get a connrct\n");
							int new_sock=accept(listen_sock,\
									(struct sockaddr*)&remote,&len);

							if(new_sock<0)
							{
								perror("accept");
								continue;
							}

							int j=1; //0 is listensock

							for(;j<SIZE;++j)
							{
								if(polls[j].fd==-1)
								{
									polls[j].fd=new_sock;
									polls[j].events=POLLIN;
									polls[j].revents=0;
									break;
								}
							}
							if(j == SIZE)
							{
								printf("full");
								close(new_sock);
								return -1;
							}
							if(j==max_num)
								max_num+=1;
						}
						else if(polls[i].fd> 0 && (polls[i].revents &\
									POLLIN))
						{
							sz=read(polls[i].fd,buf,sizeof(buf)-1);
							if(sz>0)
							{
								buf[sz]='\0';
								printf("client: %s",buf);
								write(polls[i].fd,buf,strlen(buf));
								polls[i].revents=0;
							}
							else if(sz==0)
							{
								close(polls[i].fd);
								polls[i].fd=-1;
								printf("client is close\n");
							}
						}
						else
						{
							//do noting
						}
					}
				}
			break;
		}
	}
	
	for(i=0;i<SIZE;++i)
		if(polls[i].fd==-1)
			close(polls[i].fd);

	return 0;
}


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


int main(int argc,char* argv[])
{	
	if(argc!=3)
	{
		printf("Usage %s [ip] [port]\n",argv[0]);
		exit(1);
	}


	int client_sock=socket(AF_INET,SOCK_STREAM,0);
	if(client_sock<0)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in client;
	client.sin_family=AF_INET;
	client.sin_port=htons(atoi(argv[2]));
	inet_pton(AF_INET,argv[1],&client.sin_addr);

	char buf[1024];
	ssize_t sz;

	if(connect(client_sock,(struct sockaddr*)&client,sizeof(client))<0)
	{
		perror("connect");
		exit(2);
	}

	while(1)
	{
		printf("enter: \n");
		sz=read(0,buf,sizeof(buf)-1);
		if(sz>0)
		{
			buf[sz]='\0';
		}
		write(client_sock,buf,sz);
		sz=read(client_sock,buf,sizeof(buf)-1);

		if(sz>0)
		{
			buf[sz]='\0';
			printf("server echo to client: %s\n",buf);
		}
	}
	close(client_sock);

	return 0;
}

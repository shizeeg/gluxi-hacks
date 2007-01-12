#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKETNAME "/tmp/gluxi_webstatus"

void reportForbidden(char *s)
{
	printf("Content-type: text/html\r\n");
	printf("Status: 403 Forbidden\r\n");
	printf("\r\n");
	if (s)
		printf("%s\r\n",s);
	exit(0);
}

int main(void)
{
	char *query=getenv("QUERY_STRING");
	if (!query)
		reportForbidden(0);
	if (strlen(query)>100)
		reportForbidden(0);

	struct sockaddr_un sa;
	strcpy(sa.sun_path, SOCKETNAME);
	sa.sun_family=AF_UNIX;
	int fd=socket(AF_UNIX, SOCK_STREAM, 0);
	int res=connect(fd, (struct sockaddr*)&sa, sizeof(sa));
	if (res == -1)
	{
		perror("connect:");
		reportForbidden("Can't connect to gluxi socket");
	}
	char myquery[105];
	snprintf(myquery,104,"%s\n",query);
	write(fd,myquery,strlen(myquery));
	char buf[1024];
	int bsize=0;
	int responseFinished=0;
	int i;
	buf[sizeof(buf)-1]=0;

	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);

	while (1)
	{
		tv.tv_sec=3;
		tv.tv_usec=0;
		res=select(fd+1,&rfds,NULL,NULL,&tv);
		if (res < 0)
			reportForbidden("select() failed");
		if (!res)
			reportForbidden("Client timeout");

		res=read(fd,buf+bsize,sizeof(buf)-bsize-1);
		if (res<=0)
			reportForbidden("Failed to read from gluxi socket");
		for (i=bsize; i<bsize+res; i++)
			if (buf[i]==13 || buf[i]==10)
			{
				responseFinished=1;
				buf[i]=0;
				break;
			}
		bsize+=res;
		if (responseFinished)
			break;
	}
	close(fd);

	if (!responseFinished)
		reportForbidden("Failed to read response from gluxi socket");
	if (strlen(buf)==0)
		reportForbidden("Unable to find URL");

	printf("Content-type: text/html\r\n");
	printf("Location: %s\r\n",buf);
	printf("\r\n");
}


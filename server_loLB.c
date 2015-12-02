#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>

#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include<netdb.h>

#define QCLI 20

extern int errno;
int tcp(char *port);

int main(int argc, char *argv[])
{
	struct sockaddr_in fsin;
	int msock;
	fd_set wfds;
	fd_set rfds;
	fd_set afds;
	int alen;
	int fd, nfds, rc, a;

	char *temp = malloc(sizeof(char)*1000);
	int debugflag = 0;

	msock = tcp(argv[1]);
	
	nfds = getdtablesize();
	FD_ZERO( &afds );
	FD_SET( msock, &afds );
	while(1)
	{	
		memcpy(&rfds, &afds, sizeof(rfds));
		memcpy(&wfds, &afds, sizeof(wfds));
		if(select(nfds, &rfds, &wfds, (fd_set *)0, (struct timeval*)0) < 0)
		{
			fprintf(stderr, "select: %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if(FD_ISSET(msock, &rfds))
		{
			int ssock;
			char *temp = malloc(sizeof(char)*1000);
			
			alen = sizeof(fsin);
			ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
			if(ssock < 0)
			{
				fprintf(stderr,"accept: %s.\n", strerror(errno));
				exit(EXIT_FAILURE);
			}	
			fprintf(stdout, "\nClient conneted, fd=%d.\n",ssock);
			FD_SET(ssock, &afds);

			/**  communication  **/
			char *sucMsg = "\nServer: Server connected sucess.\n\0";
			write( ssock, sucMsg, strlen(sucMsg) );
			
			continue;
		}
		
		for(fd = 0; fd < nfds; ++fd)
		{
			if(fd != msock && FD_ISSET(fd, &rfds))
			{		
				printf("debugflag=%d\n", debugflag);
				read(fd, temp, 999);
				if(strstr(temp, "disconnect") != NULL)
				{
					printf("%s\n", temp);
					close(fd);
					FD_CLR(fd, &afds);
					bzero(temp, 999);
				}
				else
				{
					printf("%s (fd=%d)", temp, fd);
					debugflag++;
					bzero(temp, 999);
				}
			}
			if(fd != msock && FD_ISSET(fd, &wfds))
			{
				//char *replyMsg = "Server reply\n\0";
				//write(fd, replyMsg , strlen(replyMsg));		
			}
		}
	}
	
	
}

int tcp(char *port)
{
	int sock, type = SOCK_STREAM;
	u_short portbase = 0;
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;
	char *protocol = "tcp\0";

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
       
	if( pse = getservbyname( port, protocol ) )
		sin.sin_port = htons( ntohs( (u_short)pse->s_port ) + portbase );
	else if( (sin.sin_port = htons( (u_short)atoi(port) ) ) == 0 )
	{
		fprintf( stderr, "can't get %s service.\n", port );
		exit(EXIT_FAILURE);
	}

	if( ( ppe = getprotobyname(protocol) ) == 0 )
	{
		fprintf(stderr, "can't get %s protocol.\n", protocol);
		exit(EXIT_FAILURE);
	}

	sock = socket( PF_INET, type, ppe->p_proto );
	if( sock < 0 )
	{
		fprintf(stderr, "can't create socket:%s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	} 
	
	if( bind( sock, (struct sockaddr *)&sin, sizeof(sin) ) < 0 )
	{
		fprintf(stderr, "can't bind to %s port:_%s.\n", port, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if( listen(sock, QCLI) < 0 )
	{
		fprintf(stderr, "can't listen on %s port:%s.\n", port, strerror(errno));
		exit(EXIT_FAILURE);
	}
        
	return sock;
	
}

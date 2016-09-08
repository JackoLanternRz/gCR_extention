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
#define MAXLINE 10000

void exec_start_monitor();
void exec_end_monitior();
int readline( int fd, char *ptr, int maxlen );
char *rm_ln_from_line( char *line );
char *rcv_cli_msg( int cli_sockfd, char *line );
int main(int argc, char *argv[])
{
	struct sockaddr_in fsin;
	int msock;
	fd_set wfds;
	fd_set rfds;
	fd_set afds;
	int alen;
	int fd, nfds, i;

	char *temp = malloc(sizeof(char)*1000);
        char *line, *rmnline;
	int debugflag = 0;

	char PORT[50], input[1000];
	FILE *pfp = fopen("scal_out_ctrl_port.txt", "r");
	while( EOF != fscanf( pfp, "%s", input) )
	{
		strcpy( PORT, input );
	}		

	msock = tcp(PORT);
	
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
			
			alen = sizeof(fsin);
			ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
			if(ssock < 0)
			{
				fprintf(stderr,"accept: %s.\n", strerror(errno));
				exit(EXIT_FAILURE);
			}	
			fprintf(stdout, "\nClient conneted, fd=%d.\n",ssock);
			FD_SET(ssock, &afds); //** set FD_SET **//
			printf("%d\n", ssock);
			
			/**  communication  **/
			//char *sucMsg = "\nServer: Server connected sucess.\n\0";
			//write( ssock, sucMsg, strlen(sucMsg) );
			
			continue;
		}
		
		for(fd = 0; fd < nfds; ++fd)
		{
			if(fd != msock && FD_ISSET(fd, &rfds))
			{		
				//printf("debugflag=%d\n", debugflag);

				line = rcv_cli_msg( fd, temp );
				printf("%s\n", line );
				rmnline = rm_ln_from_line( line );
				if(strstr( line, "disconnect") != NULL)
				{
					//printf("%s\n", rmnline);
					(void) close(fd);
					FD_CLR(fd, &afds);
					bzero(temp, 999);
				}
				if(strstr( line, "start") != NULL)
				{
					exec_start_monitor();
				}
				if(strstr( line, "end") != NULL)
				{
					exec_end_monitior();
				}
				
			}
			if(fd != msock && FD_ISSET(fd, &wfds))
			{
				;
			}
		}
	}
	
	
}

void exec_start_monitor()
{
	char argv[5][1000];
	sprintf(argv[0], "sudo");
	sprintf(argv[1], "./start.sh");
	pid_t td_pid;
	td_pid = fork();
	if( td_pid < 0 )
	{
		fprintf(stderr, "Fork start error\n");
	}
	else if( td_pid == 0 )
	{
		fprintf(stderr, "start\n");
		char * argv2[] = {argv[0], argv[1], NULL};
		execve("/usr/bin/sudo", argv2, NULL );
	}
	else
	{
		wait(NULL);
		fprintf(stderr, "End of start\n");
	}
}

void exec_end_monitior()
{
	char argv[5][1000];
	sprintf(argv[0], "sudo");
	sprintf(argv[1], "./end.sh");

	FILE *fp; 
	if( (fp = fopen("pid_of_cli","r")) != NULL )
	{
		fscanf(fp, "%s", argv[2] );
		fclose(fp);
	}

	pid_t td_pid;
	td_pid = fork();
	if( td_pid < 0 )
	{
		fprintf(stderr, "Fork end error\n");
	}
	else if( td_pid == 0 )
	{
		fprintf(stderr, "end\n");
		char * argv2[] = {argv[0], argv[1], argv[2], NULL};
		execve("/usr/bin/sudo", argv2, NULL );
	}
	else
	{
		wait(NULL);
		fprintf(stderr, "End of end\n");
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

char *rcv_cli_msg( int cli_sockfd, char *line )
{
	int n;
	n = readline( cli_sockfd, line, MAXLINE );
	if( n == 0 )
	{
		return '\0';
	}
	else if( n < 0 )
	{
		fprintf( stderr, "controller %d message error\n", cli_sockfd );
		exit(0);
	} 
	
	return line;
}

int readline( int fd, char *ptr, int maxlen )
{
	int n, rc;
	char c;
	for( n = 1; n < maxlen; n++ )
	{
		if( rc = read(fd, &c, 1) == 1 )
		{
			*ptr++ = c;
			
			if( c == '\n' )
				break;
		}
		else if( rc == 0 )
		{
			if( n == 1 )
				return 0; // EOF, no data read
			else
				break;	// EOF, some data was read
		}
		else
		{
			return -1;
		}
	}
	*ptr = 0;
	return n;
}

char *rm_ln_from_line( char *line )
{
	int i;
	for( i = 0 ; i < strlen(line) ; i++ )
	{
		if( line[i] == '\n' )
		{
			line[i] = '\0';
			break;
		}
	}
	return line;
}

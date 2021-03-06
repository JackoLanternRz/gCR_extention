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
#define SCALOUTCPU 76
#define SCALINCPU 30
#define MAXLINE 10000
#define MAX_CTRL 100
#define CPU_SHOCK 15
#define OVER_THRESH_CELL 4

extern int errno;
int tcp(char *port);
void print_float_array( float *array, int length );
void exec_GA(int ctrln);
void exec_scal_out();
void exec_scal_in();
void print_array_avg( float *array, int length );
/*void test_write( int *ctrlsockfds )
{
	int i;
	for( i = 0; i < MAX_CTRL; i++ )
	{
		if( ctrlsockfds[i] != 0 )
			write( ctrlsockfds[i], "GIVE me switch info\n" , strlen("GIVE me switch info\n") );
	}
}*/

float average( float *array, int length )
{
	int i;
	float sum = 0;
	for( i = 0; i < length; i++ )
		sum += array[i];
	
	return (sum/length);
}

int check_balance( float avg, float *array, int length )
{
	int i, not_bal = 0;
	for( i = 0; i < length; i++ )
	{
		// CPU shock = 15%
		if( (array[i] - avg) >= CPU_SHOCK || (array[i] - avg) <= -CPU_SHOCK )
			not_bal++;
	}

	if( not_bal >= OVER_THRESH_CELL )
		return 1;
	else
		return 0;

}

int not_balance( float *array, int length )
{
	float avg;
	avg = average( array, length);

	return check_balance( avg, array, length);
}

int getctrlindex( int *ctrlsockfds )
{
	int i=0;
	while( 1 )
	{
		if( ctrlsockfds[i] == 0 )
		{
			return i;
		}
		else
			i++;
	}
}

void add_ctrl_fd( int *ctrlsockfds, int ctrlindex, int ctrlfd )
{
	ctrlsockfds[ctrlindex] = ctrlfd;
}

void rm_ctrl_fd( int *ctrlsockfds, int ctrlfd )
{
	int i;
	for( i = 0 ; i < MAX_CTRL ; i++ )
	{
		if( ctrlsockfds[i] == ctrlfd )
			ctrlsockfds = 0;
	}
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

int main(int argc, char *argv[])
{
	struct sockaddr_in fsin;
	int msock;
	fd_set wfds;
	fd_set rfds;
	fd_set afds;
	int alen;
	int ctrlsockfds[MAX_CTRL];
	int fd, nfds, rc, a, i, ctrlnum = 0, uti_count = 0;
	int isnt_bal = 0, GA_running_lock = 0, init_stat = 1, sw_info_ctr = 0 ;
	float array_uti[100];
	for( i = 0 ; i < 100 ; i++ )
		array_uti[i] = 0;

	char *temp = malloc(sizeof(char)*1000);
        char *line, *rmnline;
	int debugflag = 0;

	if( argv[1] == NULL )
	{
		printf("./<file> <port>");
		exit(-1);
	}		

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
			//add_ctrl_fd( ctrlsockfds, getctrlindex( ctrlsockfds ), ssock);
			ctrlnum ++;	//** linked controller number +1 **//

			/**  communication  **/
			char *sucMsg = "\nServer: Server connected sucess.\n\0";
			write( ssock, sucMsg, strlen(sucMsg) );
			
			continue;
		}
		
		for(fd = 0; fd < nfds; ++fd)
		{
			if(fd != msock && FD_ISSET(fd, &rfds))
			{		
				//printf("debugflag=%d\n", debugflag);

				line = rcv_cli_msg( fd, temp );
				rmnline = rm_ln_from_line( line );
				if(strstr( line, "disconnect") != NULL)
				{
					printf("%s\n", rmnline);
					(void) close(fd);
					FD_CLR(fd, &afds);
					//rm_ctrl_fd( ctrlsockfds, fd );
					ctrlnum--;
					bzero(temp, 999);
				}
				else
				{
					//printf("%s\n", rmnline);
					debugflag++;
		
					char *cindex, *uti;
					
					//** record CPU utl **//
					if( ctrlnum >= 3 )
					{
						cindex = strtok( rmnline, " " );
						uti = strtok( NULL, " " );
						printf( "%s ", cindex );
						if( strcmp(uti, "inf" ) != 0 )
						{
							//printf("%f\n", atof(uti) );

							if( uti_count >= 15 && init_stat == 1 )
								init_stat = 0;

							if( uti_count >= 15 ) //** record 2.5 sec uti a set **//
							{
								uti_count = 0;
							}
							array_uti[uti_count] = atof(uti);
							uti_count ++;
							
							print_float_array(array_uti, 15);
							print_array_avg(array_uti, 15);
							if( average(array_uti, 15) >= SCALOUTCPU )
							{
								exec_scal_out();
								sleep(5);
							}
							if( (average(array_uti, 15) <= SCALINCPU) && (ctrlnum > 3) && (init_stat == 0) )
							{
								exec_scal_in();
								sleep(5);	
							}
							
							if( init_stat == 0 ) // array isn't in initial state, start count balanced
								isnt_bal = not_balance(array_uti, 15);

							if( isnt_bal == 1 && GA_running_lock == 0 )
								printf("Not balanced!\n");

							if( isnt_bal == 1 && GA_running_lock == 0 )
							{
								printf("start GA!\n");
								GA_running_lock = 1;
								exec_GA( ctrlnum );
								sleep(5);
								GA_running_lock = 2;
								isnt_bal = 0;
							}
							else ;
							
							
						}
						else
						{
							printf("dont get\n");
						}
						
					}
					//** end of record CPU utl **//
					
					bzero(temp, 999);
				}
			}
			if(fd != msock && FD_ISSET(fd, &wfds))
			{
				;
			}
		}
	}
	
	
}

void exec_GA( int ctrlnum )
{
	int i;
	char argv[10][100];
	
	sprintf(argv[0], "sudo");
	sprintf(argv[1], "./GA_connection_scal");
	sprintf(argv[2], "192.168.1.30");
	sprintf(argv[4], "%d", ctrlnum );	

	FILE *fp = fopen("port.txt", "r" );
	fscanf(fp, "%s", argv[3] );
	fclose(fp);

	pid_t td_pid;
	td_pid = fork();
	if( td_pid < 0 )
	{
		fprintf(stderr, "Fork GA_connection error\n");
	}
	else if( td_pid == 0 )
	{
		fprintf(stderr, "GA_connection.\n");
		char * argv2[] = {argv[0], argv[1], argv[2], argv[3], argv[4], NULL};
		execve("/usr/bin/sudo", argv2, NULL );
	}
	else
	{
		wait(NULL);
		fprintf(stderr, "End of GA\n");
	}
}

void exec_scal_out()
{
	sprintf(argv[0], "sudo");
	sprintf(argv[1], "./send_scal_out_signal_client");

	pid_t td_pid;
	td_pid = fork();
	if( td_pid < 0 )
	{
		fprintf(stderr, "Fork scal_out_cli error\n");
	}
	else if( td_pid == 0 )
	{
		fprintf(stderr, "scal_out\n");
		char * argv2[] = {argv[0], argv[1], NULL};
		execve("/usr/bin/sudo", argv2, NULL );
	}
	else
	{
		wait(NULL);
		fprintf(stderr, "End of scal_out\n");
	}
}

void exec_scal_in()
{
	sprintf(argv[0], "sudo");
	sprintf(argv[1], "./send_scal_in_signal_client");

	pid_t td_pid;
	td_pid = fork();
	if( td_pid < 0 )
	{
		fprintf(stderr, "Fork scal_out_cli error\n");
	}
	else if( td_pid == 0 )
	{
		fprintf(stderr, "scal_out\n");
		char * argv2[] = {argv[0], argv[1], NULL};
		execve("/usr/bin/sudo", argv2, NULL );
	}
	else
	{
		wait(NULL);
		fprintf(stderr, "End of scal_out\n");
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

void print_float_array( float *array, int length )
{
	int i;
	for( i = 0 ; i < length ; i++ )
	{
		printf(" %.2f", array[i] );
	}
	printf("\n");
}

void print_array_avg( float *array, int length )
{
	printf("avg = %.2f\n", average( array, length ) );
}

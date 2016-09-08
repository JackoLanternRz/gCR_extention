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

int main()
{   
	int sock;
	struct sockaddr_in dest;

	char *temp = malloc(sizeof(char)*1000);
	char input[1000];
	char SERVER_ADDR[50], PORT[50];
	FILE *ifp = fopen("scal_out_ctrl_ip.txt", "r");
	while( EOF != fscanf( ifp, "%s", input) )
	{
		strcpy( SERVER_ADDR, input );
	}

	FILE *pfp = fopen("scal_out_monitor_port.txt", "r");
	while( EOF != fscanf( pfp, "%s", input) )
	{
		strcpy( PORT, input );
	}

	/*---Open socket for streaming---*/
	if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		exit(errno);
	}

	/*---Initialize server address/port struct---*/
	bzero(&dest, sizeof(dest));
	dest.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	dest.sin_family = AF_INET;
	dest.sin_port = htons((u_short)atoi(PORT));
	

	/*---Connect to server---*/
	if ( connect(sock, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
	{
		perror("Connect ");
		exit(errno);
	}

	/*---call monitor to start---*/
	int val = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, val | O_NONBLOCK);
	
	while(1)
	{
		//if( read(sock, temp, 999) )
		//{	
		//	printf("\n%s\n",temp);
			write(sock, "end\n", strlen("end\n") );
			sleep(1);
			char clsConMsg[40];
			snprintf(clsConMsg, sizeof(clsConMsg), "Cli disconnect\n");
			write(sock, clsConMsg, strlen(clsConMsg));
			printf("Close connection\n");
			close(sock);
			break;
		//}

	}
	printf("Shutting down client...\n");

	return 0;
}

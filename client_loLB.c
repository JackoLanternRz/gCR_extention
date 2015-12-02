#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>

#define GET_CPU_USAGE_DELAY 500000
static int ctrl_c = 0;
float getCpuUsage(int delay);
void catchSig(int dummy);

int main(int argc, char *argv[])
{
	int sock, rc, wc;
	struct sockaddr_in server;
	char *temp = malloc(sizeof(char)*1000);
	
	int debugflag = 0;
	int ctrlN;
	
	if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
	{
		printf("Usage: ./<appname> <server ip> <server port> <index of client>\n");
		exit(EXIT_FAILURE);
	}	
	
	ctrlN = atoi(argv[3]);
	signal(SIGINT, catchSig);	

	//printf("Setting up env...\nController # :");
	//scanf("%d", &ctrlN);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		fprintf(stderr, "Could not create socket.\n");
		exit(EXIT_FAILURE);
	}
	else 
	{	fprintf(stdout, "Socket created,\n");	}
	
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons((u_short)atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		fprintf(stderr, "Connect failed, error.\n");
		exit(EXIT_FAILURE);
	}
	else
	{	fprintf(stdout, "Conneted to server.\n");	}

	int val = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, val | O_NONBLOCK);
	
	while(1)
	{
		if( read(sock, temp, 999) )
		{	
			printf("%s\n",temp);
			bzero(temp, 999);
		}

		if( ctrl_c == 0 )
		{
			float usage = getCpuUsage(GET_CPU_USAGE_DELAY);
			char usageMsg[30];
		        snprintf(usageMsg, sizeof(usageMsg), "Ctrl #%d CPU %0.2f", ctrlN, usage);
        		write(sock, usageMsg, strlen(usageMsg));
			debugflag++;
		}
		else // received ctrl+c
		{	
			char clsConMsg[40];
			snprintf(clsConMsg, sizeof(clsConMsg), "Ctrl #%d disconnect", ctrlN);
			write(sock, clsConMsg, strlen(clsConMsg));
			printf("Close connection\n");
			close(sock);
			break;
		}
		
		
	}
	printf("Shutting down client...\n");
	return 0;

}

float getCpuUsage(int delay)
{
    float a[4], b[4], load;
    FILE *fp;

    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %f %f %f %f", &a[0], &a[1], &a[2], &a[3]);
    fclose(fp);

    usleep(delay);

    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %f %f %f %f", &b[0], &b[1], &b[2], &b[3]);
    fclose(fp);

    load = ( (b[0] + b[1] + b[2]) - (a[0] + a[1] + a[2]) ) / ( (b[0] + b[1] + b[2] + b[3]) - (a[0] + a[1] + a[2] + a[3]) );

    printf("\rCPU Usage: %0.2f%%  ", load * 100);
    fflush(stdout);

    return load * 100;
}

void catchSig(int dummy)
{
	ctrl_c = 1;
}

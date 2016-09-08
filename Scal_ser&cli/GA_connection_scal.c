#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h> 
#include<math.h>

#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h>

void DEMO_FILE( int sw_paknum[][4], int org_swpaknum[][4], int swn );
int sock_to_connect_serv( char *IP, char *port );
int dfa( int sw_paknum[][4], int ctrln, int swn, int org )
{
	int i, *c_cost, sum=0, avg=0, D=0;
	c_cost =  malloc( sizeof(int)*(ctrln+1) );	
	for( i = 0 ; i < swn ; i++ )
	{
		c_cost[ sw_paknum[i][2] ] = c_cost[ sw_paknum[i][2] ] + sw_paknum[i][1];
		c_cost[ sw_paknum[i][3] ] = c_cost[ sw_paknum[i][3] ] + sw_paknum[i][1];	
	}	
	for( i = 1 ; i <= ctrln ; i++ )
	{
		sum = sum + c_cost[i];
	}
	//printf( "sum = %d\n", sum );
	avg = (int)(sum/ctrln);	
	if( org == 1 )	
		printf( "avg = %d\n", avg );

	for( i = 1 ; i <= ctrln ; i++ )
	{	
		// controller index starts from 1
		if( org == 1 )
			printf("c[%d]=%d ", i, c_cost[i] );
		D = D + abs(c_cost[i] - avg);
	}
	if( org == 1 )
		printf("\n");
	return D;
}

int rand_a_c(int ctrln, int had_c)
{
	int n;
	//printf("had_c=%d ", had_c);
	while(1)
	{
		if( ( n = ((rand()%ctrln)+1) ) != had_c )
		{
			//printf("n=%d\n", n);
			return n;
		}
	}
}

int main(int argc, char *argv[] )
{
	// dif dimansion(D) D1 == switch number, D2-1 == switch ID, D2-2 == switch packet,
	//  D2-3up == linked controller index 
	int sw_paknum[20][4];
	char input[1000], *str_sw_pkn, *str_ctrln, *str_swn;
	int i, j, k, l, ctrln, swn=20, Dold=0;

	// Connection vars	
	int sock, rc, wc;
	char *temp = malloc(sizeof(char)*1000);
	// End of connection var

	if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
	{
		printf("Usage: ./<appname> <server ip> <server port> <current open switch number>\n");
		exit(EXIT_FAILURE);
	}
	ctrln = atoi(argv[3]);
	sock = sock_to_connect_serv( argv[1], argv[2] );
	
	int val = fcntl(sock, F_GETFL, 0), hello = 0, pkt_dt_req = 0, n;

	fcntl(sock, F_SETFL, val | O_NONBLOCK);
	/**
	ctrln = 0;
	swn = 0;
	Dold = 0;
	FILE *fp = fopen( argv[1], "r");

	// get file data
	fscanf(fp, "%s", input );	//printf("%s\n", input );
	str_ctrln = strtok( input, "," ) ;
	ctrln = atoi( str_ctrln );
	str_swn = strtok( NULL, "," ) ;
	swn = atoi( str_swn );		//printf( "c=%d s=%d\n", ctrln, swn );
	i = 0;
	while( EOF !=  fscanf(fp, "%s", input ) )
	{
		input[strlen(input)] = '\0';	//printf("%s\n", input);
		j = 0;
		str_sw_pkn = strtok(input, ",");
		while( str_sw_pkn != NULL )
		{
			sw_paknum[j][i]	= atoi( str_sw_pkn );	//printf("%d ", sw_paknum[j][i] );
			j++;
			str_sw_pkn = strtok(  NULL , "," );
		}	
		
		i++;
	}
	// end of get file data
	**/	
	while(1)
	{
		if( ( n = read(sock, temp, 999) ) > 0 )
		{
			printf("%s\n",temp);
			if( strstr(temp, "sent") != NULL )
			{
				//pkt_dt_req = 0;
				//** parse data	
				char *pkt_num, *ctrl_s1, *ctrl_s2;			
				pkt_num = strtok( temp, "\n" );
				ctrl_s1 = strtok( NULL, "\n" );
				ctrl_s2 = strtok( NULL, "\n" );
			
				char *pkt_num_frag;
				i=0;
				pkt_num_frag = strtok(pkt_num, ",");
				while( pkt_num_frag != NULL )
				{
					sw_paknum[i][1] = atoi(pkt_num_frag);
					i++;
					pkt_num_frag = strtok( NULL, "," );
				}

				char *ctrl_s1_frag;
				i=0;
				ctrl_s1_frag = strtok(ctrl_s1, ",");
				while( ctrl_s1_frag != NULL )
				{
					sw_paknum[i][2] = atoi(ctrl_s1_frag);
					i++;
					ctrl_s1_frag = strtok( NULL, "," );
				}

				char *ctrl_s2_frag;
				i=0;
				ctrl_s2_frag = strtok(ctrl_s2, ",");
				while( ctrl_s2_frag != NULL )
				{
					sw_paknum[i][3] = atoi(ctrl_s2_frag);
					i++;
					ctrl_s2_frag = strtok( NULL, "," );
				}
				//** end of parse data

				for( i = 0 ; i < swn ; i++ )
					printf("%d/%d/%d ", sw_paknum[i][1], sw_paknum[i][2], sw_paknum[i][3] );	
				printf("\n");
	
				Dold = dfa( sw_paknum, ctrln, swn, 1 );	printf( "Dold = %d\n\n", Dold );
	
				/*
					chrome[][][]	record a gen's set
					chrome_D[]	record a set's dfa
					cost_of_sh[]	record how many phase a set take to finish
					Df[]		record a set's fitness value
		
					good_chr[][][]	record sets that has in bound fitness value
					good_Df[]	record good_chr sets' fitness value
	
					gen 		record current generation
					max_gen		maximun generation this GA could go
					chrome_n	calculate a generation children chromes' amount, also used as chrome index in the generation
					max_chrome	chrome quantity limitation of one generation 
					good_chr_n	calculate good chromes' amount, also used as chrome index in good chrome sets
					max_good_chr	the wanted good chrome number from all generation
				*/
				int max_chrome=10000, chrome_n=0, chrome[10000][20][4], gen=1, chrome_D[10000], p, cost_of_sh[10000], good_chr_n=0;
				int max_good_chr=4000, max_gen=(max_chrome/1000), good_chr[4000][20][4]; 
				float Df[100000], good_Df[4000];
	
	
				srand(time(NULL));
				while( chrome_n < 1000/*(int)(max_chrome/10)*/ )
				{
					/*** Initialize a chrome  ***/
					for( i = 0 ; i < 2 ; i++ )
						for( j = 0 ; j < swn ; j++ )	
							chrome[chrome_n][j][i] = sw_paknum[j][i];
		
					for( i = 2 ; i < 4 ; i++ )
						for( j = 0 ; j < swn ; j++ )
							chrome[chrome_n][j][i] = 0;
					/** END **/
		
					p = rand() % 10;
					for( j = 0 ; j < swn ; j++ )
					{
						if( p <= 3 )
						{
							chrome[chrome_n][j][2] = ( rand() % ctrln ) + 1 ;
							chrome[chrome_n][j][3] = ( ( chrome[chrome_n][j][2] + 1 ) % ctrln ) + 1;
							cost_of_sh[chrome_n] = 1;
						}
						else if( p > 3 && p <= 6 )
						{
							chrome[chrome_n][j][2] = sw_paknum[j][2];
							chrome[chrome_n][j][3] = rand_a_c( ctrln, sw_paknum[j][2] );
							cost_of_sh[chrome_n] = 0;
						}
						else if( p > 6 && p <= 9 )
						{
							chrome[chrome_n][j][3] = sw_paknum[j][3];
							chrome[chrome_n][j][2] = rand_a_c( ctrln, sw_paknum[j][3] );
							cost_of_sh[chrome_n] = 0;
						}
					}
					chrome_D[chrome_n] = dfa( chrome[chrome_n], ctrln, swn, 0 );
		
					//printf("D_fit=%f\n", ( (float)(Dold - chrome_D[chrome_n]) / Dold ) );
					Df[chrome_n] = ( (float)(Dold - chrome_D[chrome_n]) / Dold ) + cost_of_sh[chrome_n]*0.12;		

					if( Df[chrome_n] >=0 && Df[chrome_n] <= 1  )
					{
						/*for( i = 0 ; i < swn ; i++ )
							printf("%d ", chrome[chrome_n][i][2] );	
						printf("\n");
						for( i = 0 ; i < swn ; i++ )
							printf("%d ", chrome[chrome_n][i][3] );	
						printf("\n");
						printf("D_fit=%f, D=%d, cost_of_sh=%d\n", Df[chrome_n], chrome_D[chrome_n], cost_of_sh[chrome_n] );*/

						for( j = 0 ; j < 4 ; j++ )
						{			
							for( i = 0 ; i < swn ; i++ )
							{
								good_chr[good_chr_n][i][j] = chrome[chrome_n][i][j];		
							}	
						}
						good_Df[good_chr_n] = (float)Df[chrome_n];
						good_chr_n++;
					}
					chrome_n++; // go to next chrome
				}
				printf("good rate on gen 1 = %d\n", good_chr_n );
				gen++;

				// Crossover, mutation
				int parent_index[2], p_mu, mut_rate=2;	
				while( gen <= max_gen && good_chr_n < max_good_chr )
				{
					if( (chrome_n / 1000) > (gen-1) )
					{
						printf("chrome_n = %d\n", chrome_n );
						gen++;
						printf("Generation add, G=%d\n", gen );			
					}

					/*** Initialize a chrome  ***/
					for( i = 0 ; i < 2 ; i++ )
						for( j = 0 ; j < swn ; j++ )	
							chrome[chrome_n][j][i] = sw_paknum[j][i];
		
					for( i = 2 ; i < 4 ; i++ )
						for( j = 0 ; j < swn ; j++ )
							chrome[chrome_n][j][i] = 0;
					/** END **/
		
					p_mu = rand() % 10;
					if( p_mu > mut_rate )	// Crossover
					{
						/*** Select parent ***/
						for( i = 0 ; i < 2 ; i++ )
							parent_index[i] = (rand() % 1000) + (gen-1-1)*1000;  // -1 parent gen, -1 start from gen=1;
						/** END **/

						/*** Crossover produce child ***/
						for( i = 0 ; i < swn ; i++ )
						{
							chrome[chrome_n][i][2] = chrome[parent_index[(rand()%2)]][i][2];
							chrome[chrome_n][i][3] = chrome[parent_index[(rand()%2)]][i][3];
							if( chrome[chrome_n][i][3] == chrome[chrome_n][i][2] )
							{
								chrome[chrome_n][i][3] = rand_a_c( ctrln, chrome[chrome_n][i][2] );
							}
						}
						/** END **/
					}
					else			// Mutation
					{
						// Copy last chrome
						for( j = 2 ; j < 4 ; j++ )
							for( i = 0 ; i < swn ; i++ )
								chrome[chrome_n][i][j] == chrome[chrome_n-1][i][j];
			
						p = rand() % 10;
						for( j = 0 ; j < swn ; j++ )
						{
							if( p <= 3 )
							{
								chrome[chrome_n][j][2] = ( rand() % ctrln ) + 1 ;
								chrome[chrome_n][j][3] = ( ( chrome[chrome_n][j][2] + 1 ) % ctrln ) + 1;
							}
							else if( p > 3 && p <= 6 )
							{
								chrome[chrome_n][j][2] = chrome[chrome_n-1][j][2];
								chrome[chrome_n][j][3] = rand_a_c( ctrln, chrome[chrome_n-1][j][2] );
							}
							else if( p > 6 && p <= 9 )
							{
								chrome[chrome_n][j][3] = chrome[chrome_n-1][j][3];
								chrome[chrome_n][j][2] = rand_a_c( ctrln, chrome[chrome_n-1][j][3] );
							}
						}
					}			// END
		

					/*** Check cost of shifting ***/
					int dif1=0, dif2=0, dif3=0, dif4=0;
					cost_of_sh[chrome_n] = 1;
					for( i = 0 ; i < swn ; i++ )
					{
						if( chrome[chrome_n][i][2] != sw_paknum[i][2] )
							dif1 = 1;
						if( chrome[chrome_n][i][3] != sw_paknum[i][2] )
							dif2 = 1;
						if( chrome[chrome_n][i][2] != sw_paknum[i][3] )
							dif3 = 1;
						if( chrome[chrome_n][i][3] != sw_paknum[i][3] )
							dif4 = 1;
					}	
					if( dif1*dif2*dif3*dif4 == 0 )
						cost_of_sh[chrome_n] = 0;	
					/** END **/

					/*** Calculation Dfa & fitness value ***/
					chrome_D[chrome_n] = dfa( chrome[chrome_n], ctrln, swn, 0 );
					Df[chrome_n] = ( (float)(Dold - chrome_D[chrome_n]) / Dold ) + cost_of_sh[chrome_n]*0.12;
					/** END **/		

					/*** Elite record ***/
					if( Df[chrome_n] >=0 && Df[chrome_n] <= 1  )
					{
						/*for( i = 0 ; i < swn ; i++ )
							printf("%d ", chrome[chrome_n][i][2] );	
						printf("\n");
						for( i = 0 ; i < swn ; i++ )
							printf("%d ", chrome[chrome_n][i][3] );	
						printf("\n");
						printf("D_fit=%f, D=%d, cost_of_sh=%d\n", Df[chrome_n], chrome_D[chrome_n], cost_of_sh[chrome_n] );*/

						for( j = 0 ; j < 4 ; j++ )
						{			
							for( i = 0 ; i < swn ; i++ )
							{
								good_chr[good_chr_n][i][j] = chrome[chrome_n][i][j];		
							}	
						}
						good_Df[good_chr_n] = (float)Df[chrome_n];
						good_chr_n++;
					}
					/** END **/

					chrome_n++;
		
					
				}
				// END of Crossover & mutation

				int max_index;
				float max_dfa=0;
				/** browse selected chrome  **/
				for( i = 0 ; i < good_chr_n ; i++ )
				{
					/*for( j = 0 ; j < swn ; j++ )
						printf("%d,", good_chr[i][j][2] );	
					printf("\n");
					for( j = 0 ; j < swn ; j++ )
						printf("%d,", good_chr[i][j][3] );	
					printf("\n");*/
					//dfa( good_chr[i], ctrln, swn, 1 );	// print final controller received packet num
					if( good_Df[i] > max_dfa )
					{
						max_dfa = good_Df[i];
						max_index = i;
					} 		
					//printf("%d, %f\n", i, good_Df[i] );
				}
				/** END **/
	
				/** print & write best set **/
				printf("best DF=%f\n", good_Df[max_index] );
				for( j = 0 ; j < swn ; j++ )
					printf( "%d,", good_chr[max_index][j][2] );	
				printf( "\n");
				for( j = 0 ; j < swn ; j++ )
					printf( "%d,", good_chr[max_index][j][3] );
				printf( "\n");

				/*********************************************************************/ DEMO_FILE( good_chr[max_index] , sw_paknum, swn );
				
				char to_serv_msg[1000], temp[1000];
				bzero(to_serv_msg, 1000);
				bzero(temp, 1000);
				strcat( to_serv_msg , "drs " );
				for( j = 0 ; j < swn ; j++ )
				{
					sprintf( temp, "%d,", good_chr[max_index][j][2] );
					strcat( to_serv_msg, temp );
					bzero(temp, 1000);
				}
				strcat( to_serv_msg, "\n\0");
				printf("%s", to_serv_msg); 
				write(sock, to_serv_msg, strlen(to_serv_msg) );
				bzero(to_serv_msg, 1000);

				strcat( to_serv_msg, "drs " );
				for( j = 0 ; j < swn ; j++ )
				{
					sprintf( temp, "%d,", good_chr[max_index][j][3] );
					strcat( to_serv_msg, temp );
					bzero(temp, 1000 );
				}
				strcat( to_serv_msg, "\n\0");
				printf("%s", to_serv_msg);
				write(sock, to_serv_msg, strlen(to_serv_msg) );
				bzero(to_serv_msg, 1000);
	
				printf( "\nEnd of GA\n");

				/** END **/	
			}
			// End of get sw pkt # and ctrl set
			if(strstr( temp, "Disconnect") != NULL)
			{	
				char clsConMsg[40];
				snprintf(clsConMsg, sizeof(clsConMsg), "Client disconnect\n");
				write(sock, clsConMsg, strlen(clsConMsg));
				printf("Close connection\n");
				close(sock);
				break;
			}	
			bzero(temp, 1000);
		}
		if( pkt_dt_req == 0 )
		{	
			char to_serv_msg[1000];
			snprintf( to_serv_msg, sizeof(to_serv_msg), "Give_me_sw_data\n" );
			write(sock, to_serv_msg, strlen(to_serv_msg) );
			pkt_dt_req = 1;
		}
		
	}
	return 0;
}

void DEMO_FILE( int sw_paknum[][4], int org_swpaknum[][4], int swn )
{
	int i, c[3][20], i1 = 0, i2 = 0, i3 = 0, j;
	for( i = 0 ; i < swn ; i++ )
	{
		if( sw_paknum[i][2] == 1 )
		{
			c[sw_paknum[i][2]-1][i1] = i;
			i1++;
		}	
		if( sw_paknum[i][2] == 2 )
		{
			c[sw_paknum[i][2]-1][i2] = i;
			i2++;
		}	
		if( sw_paknum[i][2] == 3 )
		{
			c[sw_paknum[i][2]-1][i3] = i;
			i3++;
		}	

		if( sw_paknum[i][3] == 1 )
		{
			c[sw_paknum[i][3]-1][i1] = i;
			i1++;
		}	
		if( sw_paknum[i][3] == 2 )
		{
			c[sw_paknum[i][3]-1][i2] = i;
			i2++;
		}	
		if( sw_paknum[i][3] == 3 )
		{
			c[sw_paknum[i][3]-1][i3] = i;
			i3++;
		}	
	}
	
	int check_list[3], bool_swchlink[20] = {0};
	for( i = 0 ; i < swn ; i++ )
	{
		check_list[0] = check_list[1] = check_list[2] = 0;

		check_list[sw_paknum[i][2]]++;
		check_list[sw_paknum[i][3]]++;
		check_list[org_swpaknum[i][2]]++;
		check_list[org_swpaknum[i][3]]++;

		for( j = 0 ; j < 3 ; j++ )
		{
			if( check_list[j] == 1 )
			{
				bool_swchlink[i] = 1;
				break;
			}
		}
	}

	FILE *fp = fopen("ctrl_link.demo", "w");
	for( i = 0 ; i < i1 ; i++ )
	{
		fprintf( fp, "\\\"%d\\\"", c[0][i] );
		if( i != (i1-1) )
			fprintf( fp, ",");
	}	fprintf( fp, "\n" );

	for( i = 0 ; i < i2 ; i++ )
	{
		fprintf( fp, "\\\"%d\\\"", c[1][i] );
		if( i != (i2-1) )
			fprintf( fp, ",");
	}	fprintf( fp, "\n" );

	for( i = 0 ; i < i3 ; i++ )
	{
		fprintf( fp, "\\\"%d\\\"", c[2][i] );
		if( i != (i3-1) )
			fprintf( fp, ",");
	}	fprintf( fp, "\n" );
	for( i = 0 ; i < swn ; i++ )
	{
		if( bool_swchlink[i] == 1 )
		fprintf(fp, "%d\n", i );	
	}
	fclose(fp);
}

int sock_to_connect_serv( char *IP, char *port )
{
	int sock;
	struct sockaddr_in server;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		fprintf(stderr, "Could not create socket.\n");
		exit(EXIT_FAILURE);
	}
	else 
	{	fprintf(stdout, "Socket created,\n");	}
	
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons((u_short)atoi(port));

	if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		fprintf(stderr, "Connect failed, error.\n");
		exit(EXIT_FAILURE);
	}
	else
	{	fprintf(stdout, "Conneted to server.\n");	}
	
	return sock;
}





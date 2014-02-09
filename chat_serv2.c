#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);
void color();
void printColorString(int color,char *str);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
char addr_arry[MAX_CLNT][20]; //Array adress
int colorS=41;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	struct in_addr temp_addr;//adress
	char *str_addr;//adress
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	printf("\033[%dmServer is booted\033[0m\n",32);

	while(1)
	{
		
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		/* save Address */
		temp_addr=clnt_adr.sin_addr;
		str_addr=inet_ntoa(temp_addr);
		

		/* Insert Connected Client */
		pthread_mutex_lock(&mutx);
		strcpy(addr_arry[clnt_cnt],str_addr);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		/* Threading */
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		

		printf("\n\033[%dm Connected client IP: [%s] \033[0m\n", colorS+(clnt_sock-4),  inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}
	
/* Client Handling */
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i,j;
	char msg[BUF_SIZE];
	char temp[20];

	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0){
		send_msg(msg, str_len);
		memset(&msg, 0, sizeof(msg));
	}
	
	pthread_mutex_lock(&mutx);

	/* Remove disconnected client */
	for(i=0; i<clnt_cnt; i++)
	{
		if(clnt_sock==clnt_socks[i])
		{
			strcpy(temp, addr_arry[i]);
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
				for(j=0;j<20;j++)
					addr_arry[i][j]=addr_arry[i+1][j];
			break;
		}
	}

	printf("\n\033[%dmClient disconnected : [%s] \033[0m\n", colorS+(clnt_sock-4) ,temp);//reveal disconnect
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

/* BroadCasting */
void send_msg(char * msg, int len)
{	
	int i;
	FILE *fp=fopen("script.txt","at");
	if(fp==NULL){
		puts("fail open");
		//return -1;
	}
	
	pthread_mutex_lock(&mutx);
	fputs(msg, stdout);
	fputs(msg,fp);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);

	fclose(fp);
}


void color()
{
	printf("\n");
	printf("############### Color Checker ##############\n");
    printf("\033[30m FG_BLACK(30) \033[0m\n");
    printf("\033[31m FG_RED(31) \033[0m\n");
    printf("\033[32m FG_GREEN(32) \033[0m\n");
    printf("\033[33m FG_YELLOW(33) \033[0m\n");
    printf("\033[34m FG_BLUE(34) \033[0m\n");
    printf("\033[35m FG_VIOLET(35) \033[0m\n");
    printf("\033[36m FG_VIRDIAN(36) \033[0m\n");
    printf("\033[37m FG_WHITE(37) \033[0m");
	printf("\n");
    printf("\033[40m BG_BLACK(40) \033[0m\n");
    printf("\033[41m BG_RED(41) \033[0m\n");
    printf("\033[42m BG_GREEN(42) \033[0m\n");
    printf("\033[43m BG_YELLOW(43) \033[0m\n");
    printf("\033[44m BG_BLUE(44) \033[0m\n");
    printf("\033[45m BG_VIOLET(45) \033[0m\n");
    printf("\033[46m BG_VIRDIAN(46) \033[0m\n");
    printf("\033[47m BG_WHITE(47) \033[0m\n");
	printf("#############################################\n");
	
}

void printColorString(int color,char *str)//change color
{
	printf("\033[%dm %s \033[0m",color,str);
}

/* Error Handling */
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

/* Command Method */
#define COMMAND_NUM 8
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
	
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];

/* Command Method */
int com_Ana(char *str);
void help();

int checker=0;
char cmd[10][BUF_SIZE];
char cmd_val[COMMAND_NUM][10]={"help","intro","emot","emoton","name","exit","file","clear"};
char cmd_int[10+NAME_SIZE]="";
char cmd_sn[NAME_SIZE];

/* Helper */
void printColorString(int color, char * str) ;

/* Emoticon Gener */
void emot(int s,char * ret);

/* Intro Method */
void intro(char* ret);

/* Change Name */
void cana(char* ret);

/* String Replace Method */
char* search(char* target, char* find); //String Replace
void replace(char* target, char* oldstr, char* newstr); //String Search

/* Emotion On || Off */
#define EMOTICON_NUM 15

int emotflag=0; // off
int emoton();
char emot_b[EMOTICON_NUM][BUF_SIZE]={"smile","sad","bigsmile","surprise","wink","angly","sweat","mute","kiss","nyah","shy","strange","sleepy","boring","silence"};
char emot_a[EMOTICON_NUM][BUF_SIZE]={":)",":(",":D",":O",";)",";(","(:|",":|",":*",":p",":$",":^)","|-)","|-(",":X"};

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	sprintf(name, "[%s]", argv[3]);
	strcpy(cmd_sn,argv[3]);
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

	help();
	
	/*
	if(checker==0){
		sprintf(cmd_int, "[intro,%s]", argv[3]);
		fflush(stdin);
		write(sock, cmd_int, strlen(cmd_int));
		checker=1;
	}
	*/
	
	/* Threading */
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock); 
	return 0;
}

/* Send thread mainn */
void * send_msg(void * arg)   
{
	int sock=*((int*)arg);
	int i;
	int s;
	char name_msg[NAME_SIZE+BUF_SIZE];
	char retstr[BUF_SIZE];
	char str1[30]="\033[31mHello Everybody!!\033[0m\n";
	char str2[30]="\033[35mBye Everybody!!\033[0m\n";

		
	while(1) 
	{
		/* Reset str */
		memset(&cmd, 0, sizeof(cmd));
		memset(&retstr, 0, sizeof(retstr));

		fgets(msg, BUF_SIZE, stdin);
		s=com_Ana(msg);

		switch(s)
		{
		case 0:
			help();
			break;
		case 1:
			intro(retstr);
			sprintf(name_msg,"%s %s", retstr, str1);
			write(sock, name_msg, strlen(name_msg));
			break;
		case 2:
			emot(atoi(cmd[1]),retstr);
			sprintf(name_msg,"%s %s", name, retstr);
			write(sock, name_msg, strlen(name_msg));
			break;
		case 3:
			emotflag=emoton();
			if(emotflag==1)		printf("\033[34m[Emoticon On!!]\033[0m\n");
			else	printf("\033[34m[Emoticon Off!!]\033[0m\n");
			break;
		case 4:
			cana(retstr);
			sprintf(name_msg,"%s \033[36m%s\033[0m\n", name, retstr);
			write(sock, name_msg, strlen(name_msg));
			break;
		case 5:
			sprintf(name_msg,"%s %s", name, str2);
			write(sock, name_msg, strlen(name_msg));
			close(sock);
			exit(0);
			break;
		case 6:
			printf("\033[31m\033[43mThis method doesn't run now.\033[0m\n");
			break;
		default:
			if(emotflag==1)
				for(i=0; i<EMOTICON_NUM; i++)
					replace(msg, emot_b[i], emot_a[i]);
			
			sprintf(name_msg,"%s %s", name, msg);
			write(sock, name_msg, strlen(name_msg));
			break;
		}
	}
	return NULL;
}
	
/* Read thread main */
void * recv_msg(void * arg)
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		printf(name_msg);
	}
	return NULL;
}

/* Command Analysor */
int com_Ana(char *str)
{
	int i=0,j=0;
	char cmp[BUF_SIZE];
	char *token = NULL;
	char check[]="[,]";

	for(i=0;i<COMMAND_NUM;i++){
		if(strstr(str, cmd_val[i]) != NULL) {
			
			strcpy(cmp,str);
			token = strtok( cmp, check);

			while( token != NULL )
			{
				strcpy(cmd[j],token);
				token = strtok( NULL, check);
				j++;
			}
		}
	}

	for(i=0;i<COMMAND_NUM;i++){
		if(strcmp(cmd[0], cmd_val[i]) == 0){
			return i;
		}
	}
	return -1;
}

/* Helper */
void help() 
{
	printColorString(33,"===================================================\n");
	printColorString(33,"              Chating Program Helper               \n");
	printColorString(33,"===================================================\n\n");

	printColorString(33,"===================================================\n");
	printColorString(33," 1. Use command : [Command, factor1, factor2,....] \n");
	printColorString(33," Type of Command (factor) : help(0), intro(1),     \n");
	printColorString(33,"                            emot(1), file(2),... ] \n");
	printColorString(33,"===================================================\n");
	printColorString(33," 2. Help command : [help]                          \n");
	printColorString(33,"===================================================\n");
	printColorString(33," 3. intro command : [intro]                        \n");
	printColorString(33,"===================================================\n");
	printColorString(33," 4. emot command : [emot,num(1~10)]                \n");
	printColorString(33,"===================================================\n");
	printColorString(33," 5. emoton command : [emoton, status (on || off) ] \n");
	printColorString(33,"===================================================\n");
	printColorString(33," 6. name command : [name,name changed]             \n");
	printColorString(33,"===================================================\n");
	printColorString(33," 7. exit command : [exit]                          \n");
	printColorString(33,"===================================================\n");
	printColorString(33," 8. file command : [file,filename,name]            \n");
	printColorString(33,"                   This method doesn't run now.    \n");
	printColorString(33,"===================================================\n\n");
}

/* Emoticon Gener */
void emot(int s,char* ret)
{
	switch(s)
	{
	case 1:
		strcpy(ret,"( ^.^)\n");
		break;
	case 2:
		strcpy(ret,"( *.-) \n");
		break;
	case 3:
		strcpy(ret,"( O a o)\n");
		break;
	case 4:
		strcpy(ret,"( -x-)\n");
		break;
	case 5:
		strcpy(ret,"( 0x0)\n");
		break;
	case 6:
		strcpy(ret,"( -3-)\n");
		break;
	case 7:
		strcpy(ret,"( O-O)\n");
		break;
	case 8:
		strcpy(ret,"( 03-)\n");
		break;
	case 9:
		strcpy(ret,"( *^.^*)\n");
		break;
	case 10:
		strcpy(ret,"( m_._m)\n");
		break;
	default:
		strcpy(ret,"No Emoticon Number!!\n");
		break;
	}
}

/* Intro Method */
void intro(char* ret)
{
	char str1[BUF_SIZE]="[intro,";
	char str2[5]="]";
	char temp[BUF_SIZE];
	
	sprintf(temp,"%s %s",str1,cmd_sn);
	sprintf(ret,"%s %s",temp,str2);
}

/* Change Name */
void cana(char* ret)
{
	char temp[NAME_SIZE];

	/* Save Old Name */
	strcpy(temp,cmd_sn);

	/* Save New Name */
	memset(&cmd_sn, 0, sizeof(cmd_sn));
	strcpy(cmd_sn,cmd[1]);

	/* Make New Name tag */
	sprintf(name, "[%s]", cmd_sn);

	sprintf(ret, "Change the Name : [%s] => [%s] ",temp,cmd_sn);
}

	
/* ColorPrint */
void printColorString(int color, char * str) 
{
	printf("\033[%dm%s\033[0m", color, str);
}

/* Error Handling */
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}


/* String Replace */
void replace(char* target, char* oldstr, char* newstr)
{
  char *p;
  char tmp[BUF_SIZE];

  p = search(target, oldstr);
  while(p != NULL) {
    *p = '\0';
    strcpy(tmp, p+strlen(oldstr));
    strcat(target, newstr);
    strcat(target, tmp);
    p = search(p+ strlen(newstr), oldstr);
  }
}

/* String Search */
char* search(char* target, char* find)
{
  int s1, s2;
  char* p;

  s1 = strlen(target);
  s2 = strlen(find);

  for(p=target; p<=target+s1-s2; p++) {
    if(strncmp(p, find, s2) == 0)
      return p;
  }
  return NULL;
}

 int emoton()
{
	if(strcmp(cmd[1], "on") == 0){
			return 1;
	}
	else
		return 0;
}
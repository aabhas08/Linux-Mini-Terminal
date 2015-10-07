#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<stdio.h>
#include<pwd.h>
#include<string.h>
#include<signal.h>
#include <fcntl.h>
#define  ini bzero
static  *envp[100];
void username()
{
	char host[100];
	gethostname(host,100);
	char* path = "/bin/";
	struct passwd *x;
	uid_t p=geteuid();
	x=getpwuid(p);
	printf("<%s@",x->pw_name);
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	int count=0,i;
	for(i=0;cwd[i]!='\0';i++)
		if(cwd[i]=='/')
		{	
			count++;
			if(count==3)
			{
				int j=0;
				for(i=i;cwd[i]!='\0';i++)
					cwd[j++]=cwd[i];
				cwd[j]='\0';
			}
		}
	if(count==2)
		cwd[0]='\0';
	if(count<=1)
		printf("%s:%s$>",host,cwd);
	else
		printf("%s:~%s$>",host,cwd);
}
void pinfo(int pid)
{       
	char path[100];
	sprintf(path,"/proc/%d",pid);
	printf("pid=%d\n\n",pid);
	//	int ret=chdir(path);
	//	if(ret!=0)
	//	{       
	//		printf("invalid pid\n");
	//		return ;
	//	}
	strcat(path,"/status");
	char buff[100];
	FILE *fp=fopen(path,"r");
	if(fp==NULL)
	{       printf("No stat file found\n");
		return;
	}
	int i=0;
	while(fgets(buff,sizeof(buff),fp))
	{
		i++;
		if(i==0 || i==1 || i==2)
			printf("%s",buff);
		if(strstr(buff,"VmSize")>0)
		{       
			printf("%s",buff);
			break;

		}
	}
	sprintf(path,"/proc/%d",pid);
	strcat(path,"/exe");
	int s=readlink(path,buff,sizeof(buff));
	buff[s]='\0';
	printf("Executable Path:   %s\n\n\n",buff);
}

void handle_signal(int signo)
{
	puts("");
	username();
	fflush(stdout);
}
int output=-1,input=-1;
void  convert(char *line, char **argv)
{
	while (*line != '\0') 
	{      
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0'; 
		if(line[0]=='>')
		{	
			output=1;
		}
		if(line[0]=='<')
			input=1;
		if(line[0]!='&')
			*argv++ = line;     
		while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') 
			line++;       
	}
	*argv = '\0';               
}
int queue[100000],sq=0;
char name[100000][30];
void insertq(int pid,char str[])
{
	strcpy(name[sq],str);
	queue[sq++]=pid;
}
char ret1[30];
void removeq(int pid)
{
	int i;
	for(i=0;i<sq;i++)
		if(queue[i]==pid)
			break;
	if(i==sq)
		return;
	strcpy(ret1,name[i]);
	for(i=i;i<sq;i++)
	{	
		queue[i]=queue[i+1];
		strcpy(name[i],name[i+1]);
	}
	sq--;
}
void printq()
{
	int i;
	if(sq==0)
		printf("No jobs currently running\n");
	for(i=0;i<sq;i++)
		printf("%d %d %s\n",i+1,queue[i],name[i]);
}
int gpid=-100;
void sig_handler(int signo)
{
	/*	if (signo == SIGUSR1)
		printf("received SIGUSR1\n");
		else if (signo == SIGKILL)
		printf("received SIGKILL\n");
		else if (signo == SIGSTOP)
		printf("received SIGSTOP\n");
		else if (signo == SIGINT)
		printf("received SIGINT\n");*/
	if(signo==SIGTSTP)
		if(gpid!=-100)
		{
			kill(gpid,SIGTSTP);
			gpid=-900;
		}

}
void  convert1(char *line, char **argv)
{
	while (*line != '\0') 
	{      
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0'; 
		if(line[0]=='>')
		{	
			output=1;
//			line[0]='\0';
		}
		//	if(line[0]=='|')
		//		stpipe[spipe++]=line;
		if(line[0]=='<')
		{		input=1;
		//	line[0]='\0';
		}
		if(line[0]!='&')
			*argv++ = line;     
		while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') 
			line++;       
	}
	*argv = '\0';   
}	
void mypipe(char* str)
{
	char temp[1000];
	strcpy(temp,str);
	char *arg[1000];
	int i;
	input=-1;
	output=-1;
	convert1(str,arg);
	int pipes[200][2];
	int stpipe[100],spipe=0;
	for(i=0;arg[i];i++)
	{	
		//		printf("%d %s %d %d\n",i,arg[i],input,output);
		if(arg[i][0]=='|')
		{	
			//			printf("%d %s\n",i,arg[i]);
			pipe(pipes[spipe]);
			stpipe[spipe++]=i;
			arg[i]=NULL;
		}
	}
	int length=i;
	int prev,previ;
	int stdo=dup(1);
	int stdi=dup(0);
	if(input==1)
	{
//		previ = dup(0);
		int fd=open(arg[stpipe[0]-1],O_RDONLY);
//		char te[123];
//		getcwd(te,123);
//		printf("|%s| opening file %d path=%s\n",arg[2],fd,te);
		arg[stpipe[0]-2]=NULL;
		if(fd<0)
		{	
			puts("input file missing");
			return ;
		}
		dup2(fd,0);
		close(fd);
	}
	pid_t pid;
	pid=fork();
	char *arg2[100];
	if(pid<0)
	{
		printf("Error creating process\n");
		_exit(1);
	}
	else if(pid==0)  //Child Process
	{
		int ret;
		dup2(pipes[0][1],1);
		//      printf("parent pid=%d\n",getppid());
		//      for(i=0;arg[i];i++)
		//              printf("[%d %s]  ",i,arg[i]);
		//		puts("ExeCVP");
//		int d=0;
//		if(input==1)
//			d=2;
//		for(i=0;i<stpipe[0]-d;i++)
//			arg2[i]=arg[i];
//		arg2[i]=NULL;
		//		for(i=0;arg2[i];i++)
		//			printf("[%d %s]  ",i,arg2[i]);
//		puts("");
		for(i=0;i<spipe;i++)
		{	
			close(pipes[i][0]);
			close(pipes[i][1]);
		}
		ret=execvp(arg[0],&arg[0]);
		if(ret<0)
			printf("fail to execute command :  %s\n",arg[0]);
		_exit(0);
	}
	else
	{
		//wait(&i);
		for(i=1;i<=spipe-1;i++)
		{
			pid=fork();
			if(pid==0)
			{
				dup2(pipes[i-1][0],0);
				dup2(pipes[i][1],1);
				int yy=0;
				int j;
//				for(j=stpipe[i-1]+1;j<stpipe[i];j++)
//					arg2[yy++]=arg[j];
//				arg2[j]=NULL;
				for(j=0;j<spipe;j++)
				{	
					close(pipes[j][0]);
					close(pipes[j][1]);
				}
		//		for(i=0;arg2[i];i++)
		//			printf("[%d %s]  ",i,arg2[i]);
				//int		ret=execvp(arg2[0],arg2);
				int		ret=execvp(arg[stpipe[i-1]+1],&arg[stpipe[i-1]+1]);
				if(ret<0)
					printf("fail to execute command :  %s\n",arg[0]);
				_exit(0);
			}
		}
	}
	if(fork()==0)
	{
		if(output==1)
		{
			//                              printf("EXECUTING OUTPUT %d\n",getpid());
			//prev=dup(1);
			int fd=open(arg[length-1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
	//		printf("file=%s          %s\n",arg[length-1],arg[length-2]);
			arg[length-2]=NULL;
			dup2(fd,1);
			close(fd);
		}
		dup2(pipes[i-1][0],0);
		int yy=0;
		int j;
//				for(j=stpipe[i-1]+1;j<stpipe[i];j++)
//					arg2[yy++]=arg[j];
//				arg2[j]=NULL;
//		puts("exxeasdasdasda\n");
				for(j=0;j<spipe;j++)
				{	
					close(pipes[j][0]);
					close(pipes[j][1]);
				}
//				for(i=0;arg[i];i++)
//					printf("[%d %s]  ",i,arg2[i]);
				int		ret=execvp(arg[stpipe[i-1]+1],&arg[stpipe[i-1]+1]);
				if(ret<0)
					printf("fail to execute command :  %s\n",arg[0]);
				_exit(0);
	}
	for(i=0;i<spipe;i++)
	{
					close(pipes[i][0]);
					close(pipes[i][1]);
	}
	for(i=0;i<spipe+1;i++)
	{	
		wait(NULL);
	}
/*	if(output==1)
		dup2(prev,1);
	if(input==1)
		dup2(previ,0);*/
	dup2(stdo,1);
	dup2(stdi,0);
//	puts("finish");
//	scanf("%d",&i);
//	printf("i=%d\n",i);
	username();
}
int main(int argc, char *argv[], char *envp[])
{
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);
	signal(SIGTSTP,sig_handler);
	char str[100000];
	char c=' ';
	username();
	pid_t pid;
	int a;
	int mode=0;
	int npipe=0;
	while(1) 
	{
		gpid=-100;
		do
		{
			c=getchar_unlocked();
		}while(c==EOF);
		int p=waitpid(-1,&a, WNOHANG );
		if(p>0)
		{
			removeq(p);
			if(a==0)
				//	printf("Background Child process whose pid=%d and name= %s exited normally\n",p,ret1);
				printf("%s with pid %d exited normally\n",ret1,p);
			else
				printf("%s with pid %d exited abnormally\n",ret1,p);
			//printf("Background Child process whose pid=%d and name= %s exited abnormally\n",p,ret1);
		}
		if(c=='\n')
		{
	//		printf("np=%d %d\n",npipe,pid);
			if(npipe!=0)
			{
				mypipe(str);
				ini(str,sizeof(str));
				//puts("Retuened");
			}
			else if(strlen(str)!=0 && npipe==0)
			{	
				//	printf("cmd=%s\n",str);
				char *arg[100];
				char temp1[100];
				strcpy(temp1,str);
				output=-1;
				input=-1;
				convert(str,arg);
				int prev,previ;
				if(output==1 && input!=1)
				{
					//				printf("EXECUTING OUTPUT %d\n",getpid());
					int i;
					for(i=0;arg[i];i++)
					{
						if(strcmp(arg[i],">")==0)
						{

							break;
						}
					}
					prev = dup(1);
					int fd=open(arg[i+1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					dup2(fd,1);
					close(fd);
					arg[i]=NULL;
				}
				if(input==1 && output!=1)
				{
					//		printf("EXECUTING INPUT %d\n",getpid());
					int i;
					for(i=0;arg[i];i++)
					{
						if(strcmp(arg[i],"<")==0)
						{
							break;
						}
					}
					previ = dup(0);
					int fd=open(arg[i+1],O_RDONLY);
					//     printf("|%s| opening file %d\n",arg[i+1],fd);
					dup2(fd,0);
					close(fd);
					arg[i]=NULL;
				}
				if(input==1 && output==1)
				{

					//		printf("EXECUTING OUTPUT/INPUT %d\n",getpid());
					int i;
					int stop1,stop2,s;
					for(i=0;arg[i];i++)
					{
						//if(strcmp(arg[i],"<")==0||strcmp(arg[i],">"==0))
						if(strcmp(arg[i],"<")==0)//||strcmp(arg[i],">"==0))
						{
							stop1=i;
							s=i;
							for(i=i;arg[i];i++)
								if(strcmp(arg[i],">")==0)//||strcmp(arg[i],">"==0))
								{
									stop2=i;
								}
							break;
						}
						if(strcmp(arg[i],">")==0)//||strcmp(arg[i],">"==0))
						{
							stop2=i;
							s=i;
							for(i=i;arg[i];i++)
								if(strcmp(arg[i],"<")==0)//||strcmp(arg[i],">"==0))
								{
									stop1=i;
								}
							break;
						}
					}
					previ = dup(0);
					prev=dup(1);
					int fd=open(arg[stop1+1],O_RDONLY);
					dup2(fd,0);
					close(fd);
					fd=open(arg[stop2+1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					dup2(fd,1);
					close(fd);
					arg[s]=NULL;
				}
				pid=fork();
				int i;
				if(pid<0)
				{
					printf("Error creating process\n");
					_exit(1);
				}
				else if(pid==0)  //Child Process
				{
					int ret;
					//	printf("parent pid=%d\n",getppid());
					if((strcmp(arg[0],"cd")==0)|(strcmp(arg[0],"quit")==0))
					{
						_exit(0);
					}
					if((strcmp(arg[0],"jobs")==0)|(strcmp(arg[0],"kjob")==0))
					{
						_exit(0);
					}
					if((strcmp(arg[0],"overkill")==0)|(strcmp(arg[0],"fg")==0)||(strcmp(arg[0],"pinfo")==0))
						_exit(0);
					//	for(i=0;arg[i];i++)
					//		printf("[%d %s]  ",i,arg[i]);
					//	puts("ExeCVP");
					ret=execvp(str,arg);
					if(ret<0)
						printf("fail to execute command :  %s\n",str);
					_exit(0);
				}
				else
				{
					if(output==1)
						dup2(prev,1);
					if(input==1)
						dup2(previ,0);
					gpid=pid;
					if(strcmp(arg[0],"cd")==0)
						chdir(arg[1]);
					if(strcmp(arg[0],"quit")==0)
						return 0;
					if(strcmp(arg[0],"jobs")==0)
						printq();
					if(strcmp(arg[0],"kjob")==0)
					{	
						//	printf("%d %d %d\n",sq,queue[atoi(arg[1])-1],atoi(arg[2])	);
						if(atoi(arg[1])>=sq)
							printf("invalid job no.\n");
						else
							kill(queue[atoi(arg[1])-1],atoi(arg[2]));
						//	if(atoi(arg[2])==9)
						//		removeq(queue[atoi(arg[1])-1]);

						//	printf("%d %d %d\n",sq,queue[atoi(arg[1])-1],atoi(arg[2])	);
						//			printq();
					}
					if(strcmp(arg[0],"overkill")==0)
					{
						int i;
						for(i=0;i<sq;i++)
						{
							kill(queue[i],9);
						}
					}
					if(strcmp(arg[0],"fg")==0)
					{
						if(atoi(arg[1])>sq)
							printf("invalid job no.\n");
						else
						{	
							kill(queue[atoi(arg[1])-1],SIGCONT);
							waitpid(queue[atoi(arg[1])-1],&i,0);
							removeq(queue[atoi(arg[1])-1]);
						}
					}
					if(strcmp(arg[0],"pinfo")==0)
					{
						if(arg[1])
						{
							pinfo(queue[atoi(arg[1])-1]);
						}
						else
						{
							for(i=0;i<sq;i++)
								pinfo(queue[i]);
						}
					}
					//		for(i=0;i<5;i++)
					//			printf("%d=%s\n",i,arg[i]);
					//					printf("mode=%d\n",mode);
					if (mode==0)
					{
						//	printf("\nwaiting	\n");
						//						int p=wait(&a);
						int p=waitpid(gpid,&a,WUNTRACED);
						if(WIFSTOPPED(a))
						{
							insertq(pid,temp1);
						}	

						//					printf("wait Child (%d) finished\n", p);
					}
					else
					{	
						//		printf("bp pid=%d\n",pid);
						insertq(pid,temp1);
					}
					//		int p=waitpid(-1,&a, WNOHANG );
					//		if(p>0)
					//			printf("waitpid Child (%d) finished\n", p);
					username();
					ini(str,sizeof(str));
				}
			}
			else// if(pid!=0)
			{
				//		puts("newline");
				username();
			}
			mode=0;
			npipe=0;
		}
		else
		{
			if(c=='&')
				mode=1;
			if(c=='|')
				npipe++;
			strncat(str,&c,1);
		}
	}
	puts("");
	return 0;
}

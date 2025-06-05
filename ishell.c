#include"ishell.h"

int main(int argc,char* argv[]){
        //char** chargv;
	char inbuf[MAX_BUFFER];
	pid_t pid;
	FILE* cmdlogfile;
	while(1){
		cmdlogfile=fopen("/home/uppercase/Desktop/.ishellcmdlog/cmdlogfile.txt","a+");
                if(cmdlogfile==NULL){
                        fputs("Unable to open cmdlogfile.txt\n",stderr);
                        break;
                }
                fputs("ishell$-",stdout);
		if((fgets(inbuf, MAX_BUFFER, stdin))==NULL) continue;
		inbuf[strlen(inbuf)-1]='\n';
		fputs(inbuf,cmdlogfile);
		inbuf[strlen(inbuf)-1]='\0';
		if(strcmp(inbuf, QUIT_STRING)==0)
		      exit(0);
		fclose(cmdlogfile);
                switch(pid=fork()){
                        case -1:
                                printf("Unable To Fork\n");
                                break;
                        case 0:
                                executecmd(inbuf);
				exit(1);
                        default:
                                wait(NULL);
				break;
                }
        }
        return 0;
}

int getargv(const char* s, const char* delims, char*** argvp) {
	int errrr, i, numtokens=0;
	const char* snew;
	char* t;
	if((s==NULL) || (delims==NULL) || (argvp==NULL))
	*argvp=NULL;
	snew=s+strspn(s, delims);
	if((t=malloc(strlen(snew)+1))==NULL) return -1;
	strcpy(t, snew);
	if(strtok(t, delims)!=NULL) {
		for(numtokens=1; strtok(NULL, delims)!=NULL; numtokens++);
	}
	if((*argvp=malloc((numtokens+1)*sizeof(char*)))==NULL) {
		free(t);
		return -1;
	}
	if(numtokens==0) free(t);
	else {
		strcpy(t, snew);
		**argvp=strtok(t, delims);
		for(i=1; i<numtokens; i++)
			*((*argvp)+i)=strtok(NULL, delims);
	}
	*((*argvp)+numtokens)=NULL;
	return numtokens;
}

void executecmd(char* cmds){
	int child, count, i;
	int fds[2];
	char** pipelist;
	char** chargv;
	count = getargv(cmds,"|",&pipelist);
	if(count <= 0){
		fprintf(stderr ,"Failed to find command\n");
		exit(1);
	}
	for(i=0 ;i<count-1 ;i++){
		if(pipe(fds)==-1)
			perror_exit("Failed to create pipes");
		else if((child=fork())==-1)
			perror_exit("Failed to create process to run command");
		else if(child){
			if(dup2(fds[1],STDOUT_FILENO)==-1)
				perror_exit("Failed to connect pipeline");
			if(close(fds[0]) || close(fds[1]))
				perror_exit("Failed to close needed files");
			executeredirect(pipelist[i], i==0, 0);
			exit(1);
		}
		if(dup2(fds[0], STDIN_FILENO)==-1) perror_exit("Failed to connect last component");
		if(close(fds[0]) || close(fds[1])) perror_exit("Failed to final close");
	}
	executeredirect(pipelist[i], i==0, i);
	exit(1);
}

int parseandredirectin(char* cmd){
	int error, infd;
	char* infile;
	if((infile=strchr(cmd,'<'))==NULL) return 0;
	*infile='\0';
	infile=strtok(infile+1, " \t");
	if(infile==NULL) return 0;
	if((infd=open(infile, O_RDONLY))==-1) return -1;
	if(dup2(infd, STDIN_FILENO)==-1){
		error=errno;
		close (infd);
		errno=error;
		return -1;
	}
	return close(infd);
}

int parseandredirectout(char* cmd){
	int error, outfd;
	char* outfile;
	if((outfile=strchr(cmd,'>'))==NULL) return 0;
	*outfile='\0';
	outfile=strtok(outfile+1," \t");
	if(outfile==NULL) return 0;
	if((outfd=open(outfile,FFLAG,FMODE))==-1) return -1;
	if(dup2(outfd,STDOUT_FILENO)==-1){
		error=errno;
		close(outfd);
		errno=error;
		return -1;
	}
	return close(outfd);
}

static void perror_exit(char* s){
	perror(s);
	exit(1);
}

void executeredirect(char* s ,int in ,int out){
	char** chargv;
	char* pin;
	char* pout;
	if(in &&((pin=strchr(s,'<'))!=NULL) && out && ((pout=strchr(s,'>'))!=NULL) && (pin > pout)){
		if(parseandredirectin(s)==-1){
			perror("Failed to redirect input");
			return;
		}
		in = 0;
	}
	if(out &&(parseandredirectout(s)==-1))
		perror("Failed to redirect output");
	else if(in && (parseandredirectin(s)==-1))
		perror("Failed rto redirect input");
	else if(getargv(s," \t",&chargv)<=0)
		fprintf(stderr,"Failed to parse command \n");
	else{
		execvp(chargv[0],chargv);
		perror("Failed to execute command");
	}
	exit(1);
}

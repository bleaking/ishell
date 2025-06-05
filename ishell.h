#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<errno.h>
#define FFLAG (O_WRONLY | O_CREAT | O_TRUNC)
#define FMODE (S_IRUSR | S_IWUSR)
#define MAX_BUFFER 256
#define QUIT_STRING "q"
int parseandredirectin(char*);
int parseandredirectout(char*);
void executecmd(char*);
int getargv(const char*, const char*, char***);
void  executeredirect(char* ,int ,int);
static void  perror_exit(char*);

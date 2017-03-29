#ifndef SERVER_H_
#define SERVER_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
//#include <mysql.h>

#define USER_NAME_PHASE 0
#define USER_PASS_PHASE 1
#define USER_WELCOME 2
#define USER_LOGIN 3
#define USER_HAS_PORT 4
#define USER_USE_PASV 5

#define DIR_LENGTH 512
#define BUFFER_LENGTH 1024
#define FILE_LENGTH 4096
#define FILE_INFO_LENGTH 65536
#define USER_LENGTH 256
#define MOST_USER 10

#define LEGAL_USER_NUM 20
#define COLUMN_NUM 32
#define USER_INFO_LENGTH 32
#define SQL_PORT 0

#define RENAME_START 0
#define RENAME_FINISH 1

int all_file_num = 0;
int bytes_num = 0;
int port_for_pasv[MOST_USER];
char *legal_name[LEGAL_USER_NUM];
int legal_user_num;

struct thread_arg{
	int connfd;
	char* sentence;
	int phase;
	struct sockaddr_in client;
};

struct file_thread_arg{
	struct sockaddr_in in;
	int control_socket;
	int ip_num[6];
	char* sentence;
	int* socket;
};

void changeToUpper(char* str);
void readInLegalName(char **);
void welcomeInfo(int socket);
void tackleRename(int control_socket, char * str, int *rename_start, char * oldname);
int tackleUser(char * str, int socket, char * name);
int tacklePass(char * str, int socket);
void tackleHelp(char * str, int socket);
void tackleChangeDirectory(int control_socket, char * str);
void tacklePwd(int control_socket, char * str);
void tacklePasvFile(int socket, char *str, struct sockaddr_in addr, int control_socket, char *);
char* getFileList(char * current_dir, char* file_info);
char* getShortFileList(char * current_dir, char* file_info);
void getParameter(char * request_file, char * str, int control_length);
void tackleCommonCmd(char * str, int control_socket);
void writeMyData(const char * str, int socket);
void readMyData(char * str, int socket);
void *handleClientRequest(void * arg);
void *handlePasvFileRequest(void * arg);
void *handlePortFileRequest(void * arg);
void sessionLoop(int connfd, char* sentence, int phase, struct sockaddr_in temp_addr);
void pasvLoop(struct file_thread_arg * in_port);
void portLoop(struct file_thread_arg * in_port);

#endif
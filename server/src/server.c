#include "server.h"

int main(int argc, char **argv) {
	int listenfd, connfd;
	struct sockaddr_in addr;
	char sentence[BUFFER_LENGTH];
	int phase;
	struct thread_arg arg;
	struct sockaddr_in client;
	unsigned int in_size = sizeof(struct sockaddr_in);
	pthread_t thread;
	int ftp_server_port;
	char ftp_root_dir[DIR_LENGTH];

	readInLegalName(legal_name);
	ftp_root_dir[0] = '\0';
	if(argc <= 1){
		ftp_server_port = 21;
	}
	else{
		for(int i = 1; i < argc; i++){
			if(strcmp("-port", argv[i]) == 0){
				ftp_server_port = atoi(argv[i + 1]);
			}
			if(strcmp("-root", argv[i]) == 0){
				strcpy(ftp_root_dir, argv[i + 1]);
			}
		}
	}
	if(chdir(ftp_root_dir) < 0){
		if(chdir("/tmp")){
			printf("Cannot change root directory to /tmp");
		}
	}
	for(int i = 0; i < MOST_USER; i++){
		port_for_pasv[i] = -1;
	}
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ftp_server_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	if (listen(listenfd, MOST_USER) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	phase = USER_WELCOME;
	while (1) {
		if ((connfd = accept(listenfd, (struct sockaddr *)&client, &in_size)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}

		//准备创建新线程处理请求
		arg.connfd = connfd;
		arg.sentence = sentence;
		arg.phase = phase;
		arg.client = client;
		if(pthread_create(&thread, NULL, handleClientRequest, (void*)&arg))
		{
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			writeMyData("421-ftp.heaven.af.mil is too busy to provide files right now.\r\n421 Please come back in 2040 seconds.\r\n", connfd);
			exit(1);
		}
	}
	close(listenfd);
}

void readInLegalName(char ** name_list){
	FILE * fp;
	int i;
	int len;

	i = 0;
	char temp[USER_LENGTH];
	if((fp = fopen("User.config", "r")) != NULL){
		while(i < LEGAL_USER_NUM && (fgets(temp, USER_LENGTH, fp) != NULL)){
			len = strlen(temp) + 1;
			name_list[i] = (char *)malloc(len * sizeof(char));
			strcpy(name_list[i], temp);
			name_list[i][len - 2] = '\0';
			i++;
		}
		legal_user_num = i;
		fclose(fp);
	}
	else{
		printf("Can't open user information file.");
	}

}

void *handleClientRequest(void * arg){
	struct thread_arg * ARG = (struct thread_arg *)arg;
	char data[BUFFER_LENGTH];
	strcpy(data, ARG->sentence);
	int socket = ARG->connfd;
	struct sockaddr_in ff = ARG->client;
	sessionLoop(socket, data, USER_WELCOME, ff);
	close(socket);
	pthread_exit(NULL);
}

void *handlePasvFileRequest(void * arg){
	struct file_thread_arg *ARG = (struct file_thread_arg *)arg;
	pasvLoop(ARG);
	pthread_exit(NULL);
}
void *handlePortFileRequest(void * arg){
	struct file_thread_arg *ARG = (struct file_thread_arg *)arg;
	portLoop(ARG);
	pthread_exit(NULL);
}

void welcomeInfo(int socket){
	const char* welcome = "220 Anonymous FTP server ready.\r\n";
	writeMyData(welcome, socket);
}

void tackleHelp(char * str, int socket){
	char request_command[DIR_LENGTH];
	char info[BUFFER_LENGTH];

	getParameter(request_command, str, 4);
	if(strcmp(request_command, "USER") == 0){
		writeMyData("214 Syntax: USER <user-name>. If provided no user_name, use anonymous.\r\n", socket);
	}
	else if(strcmp(request_command, "PASS") == 0){
		writeMyData("214 Syntax: PASS <e-mail>. If provided no e-mail, use "".\r\n", socket);
	}
	else if(strcmp(request_command, "PASV") == 0){
		writeMyData("214 Syntax: PASV\r\n", socket);
	}
	else if(strcmp(request_command, "PORT") == 0){
		writeMyData("214 Syntax: PORT ip_address and number\r\n", socket);
	}
	else if(strcmp(request_command, "RETR") == 0){
		writeMyData("214 Syntax: RETR file-name\r\n", socket);
	}
	else if(strcmp(request_command, "STOR") == 0){
		writeMyData("214 Syntax: STOR file-name\r\n", socket);
	}
	else if(strcmp(request_command, "APPE") == 0){
		writeMyData("214 Syntax: APPE file-name\r\n", socket);
	}
	else if(strcmp(request_command, "PWD") == 0){
		writeMyData("214 Syntax: PWD\r\n", socket);
	}
	else if(strcmp(request_command, "CWD") == 0){
		writeMyData("214 Syntax: CWD dir-name\r\n", socket);
	}
	else if(strcmp(request_command, "CDUP") == 0){
		writeMyData("214 Syntax: CDUP\r\n", socket);
	}
	else if(strcmp(request_command, "MKD") == 0){
		writeMyData("214 Syntax: MKD dir-name\r\n", socket);
	}
	else if(strcmp(request_command, "RMD") == 0){
		writeMyData("214 Syntax: RMD dir-name\r\n", socket);
	}
	else if(strcmp(request_command, "DELE") == 0){
		writeMyData("214 Syntax: DELE file-name or dir-name\r\n", socket);
	}
	else if(strcmp(request_command, "RNFR") == 0){
		writeMyData("214 Syntax: RNFR old-file-name or old-dir-name\r\n", socket);
	}
	else if(strcmp(request_command, "RNTO") == 0){
		writeMyData("214 Syntax: RNTO new-file-name or new-dir-name\r\n", socket);
	}
	else if(strcmp(request_command, "LIST") == 0){
		writeMyData("214 Syntax: LIST\r\n", socket);
	}
	else if(strcmp(request_command, "NLST") == 0){
		writeMyData("214 Syntax: NLST\r\n", socket);
	}
	else if(strcmp(request_command, "QUIT") == 0){
		writeMyData("214 Syntax: QUIT\r\n", socket);
	}
	else if(strcmp(request_command, "ABOR") == 0){
		writeMyData("214 Syntax: ABOR\r\n", socket);
	}
	else if(strcmp(request_command, "SYST") == 0){
		writeMyData("214 Syntax: SYST\r\n", socket);
	}
	else if(strcmp(request_command, "TYPE") == 0){
		writeMyData("214 Syntax: TYPE I\r\n", socket);
	}
	else if(strcmp(request_command, "MODE") == 0){
		writeMyData("214 Syntax: MODE S\r\n", socket);
	}
	else if(strcmp(request_command, "STRU") == 0){
		writeMyData("214 Syntax: STRU F\r\n", socket);
	}	
	else if(strcmp(request_command, "ALLO") == 0){
		writeMyData("214 Syntax: ALLO\r\n", socket);
	}
	else{
		snprintf(info, BUFFER_LENGTH, "502 Unknown command %s.\r\n", request_command);
		writeMyData(info, socket);
	}
}

int tackleUser(char * str, int socket, char * name){
	getParameter(name, str, 4);
	if (name[0] == '\0'){
		strcpy(name, "anonymous");
	}
	if(strstr(str, "USER") == str){
		if(strlen(str) == 4){
			writeMyData("331 Guest login ok, send your complete e-mail address as password.\r\n", socket);
			return 1;
		}
		for(int i = 0; i < legal_user_num; i++){
			if(strcmp(name, legal_name[i]) == 0){
				writeMyData("331 Guest login ok, send your complete e-mail address as password.\r\n", socket);
				return 1;
			}
		}
		writeMyData("530 User cannot log in.\r\n", socket);
		return 0;
	}
	else{
		writeMyData("502 The server does not support the verb or you use the verb in the wrong phase.\r\n", socket);
		return 0;
	}
}

int tacklePass(char * str, int socket){
	if(strstr(str, "PASS") == str){
		if(strlen(str) == 4){
			writeMyData("230-\r\n230-Welcome to\r\n230- School of Software\r\n230- FTP Archives at ftp.CaoZhangjie.org\r\n230-\r\n230-This site is provided as a public service by School of\r\n230-Software. Use in violation of any applicable laws is strictly\r\n230-prohibited. We make no guarantees, explicit or implicit, about the\r\n230-contents of this site. Use at your own risk.\r\n230-\r\n230 Guest login ok, access restrictions apply.\r\n", socket);
			return 1;
		}
		char * at = strstr((str + 5), "@");
		if(at != NULL){
			writeMyData("230-\r\n230-Welcome to\r\n230- School of Software\r\n230- FTP Archives at ftp.CaoZhangjie.org\r\n230-\r\n230-This site is provided as a public service by School of\r\n230-Software. Use in violation of any applicable laws is strictly\r\n230-prohibited. We make no guarantees, explicit or implicit, about the\r\n230-contents of this site. Use at your own risk.\r\n230-\r\n230 Guest login ok, access restrictions apply.\r\n", socket);
			return 1;
		}
		else{
			writeMyData("530 User cannot log in.\r\n", socket);
			return 0;
		}
	}
	else{
		writeMyData("502 The server does not support the verb or you use the verb in the wrong phase.\r\n", socket);
		return 0;
	}
}

void tacklePwd(int control_socket, char * str){
	char current_dir[DIR_LENGTH];
	getcwd(current_dir, DIR_LENGTH);
	char info[BUFFER_LENGTH];
	snprintf(info, BUFFER_LENGTH, "257 \"%s\" is current directory.\r\n", current_dir);
	writeMyData(info, control_socket);
}

void tackleChangeDirectory(int control_socket, char * str){
	char current_dir[DIR_LENGTH];
	char request_file[DIR_LENGTH];
	char buffer[BUFFER_LENGTH];
	char info[BUFFER_LENGTH];
	int control_length;
	getcwd(current_dir, DIR_LENGTH);

	control_length = strlen(str);
	if(control_length >= 4){
		getParameter(request_file, str, 3);
		snprintf(buffer, DIR_LENGTH, "%s/%s", current_dir, request_file);
	}
	if(strstr(str, "CWD") == str){
		if(chdir(buffer) >= 0){
			strcpy(info, "250 CWD command successful.\r\n");
			writeMyData(info, control_socket);
		}
		else if(chdir(request_file) >= 0){
			strcpy(info, "250 CWD command successful.\r\n");
			writeMyData(info, control_socket);
		}
		else{
			snprintf(info, BUFFER_LENGTH, "550 %s: No such file or directory.\r\n", request_file);
			writeMyData(info, control_socket);
		}
	}
	else if(strstr(str, "CDUP") == str){
		if(strcmp(current_dir, "/") == 0){
			strcpy(info, "550 Current directory is already the root directory.\r\n");
			writeMyData(info, control_socket);
		}
		else{
			char * lice_pos = strrchr(current_dir, '/');
			(*lice_pos) = '\0';
			if(chdir(current_dir) >= 0){
				strcpy(info, "250 CWD command successful.\r\n");
				writeMyData(info, control_socket);
			}
			else{
				strcpy(info, "550 CWD command failed.\r\n");
				writeMyData(info, control_socket);
			}
		}
	}
	else if(strstr(str, "MKD") == str){
		if(mkdir(buffer, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH) >= 0){
			writeMyData("250 The directory is made successfully.", control_socket);
		}
		else if(mkdir(request_file, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH) >= 0){
			writeMyData("250 The directory is made successfully.", control_socket);
		}
		else{
			writeMyData("550 The creation failed.", control_socket);
		}
	}
	else if(strstr(str, "RMD") == str){
		if(rmdir(buffer) >= 0){
			writeMyData("250 The directory is removed successfully.", control_socket);
		}
		else if(rmdir(request_file) >= 0){
			writeMyData("250 The directory is removed successfully.", control_socket);
		}
		else{
			writeMyData("550 The removal failed.", control_socket);
		}
	}
	else if(strstr(str, "DELE") == str){
		getParameter(request_file, str, 4);
		if(remove(buffer) >= 0){
			writeMyData("250 The file is removed successfully.", control_socket);
		}
		else if(remove(request_file) >= 0){
			writeMyData("250 The file is removed successfully.", control_socket);
		}
		else{
			writeMyData("550 The removal failed.", control_socket);
		}
	}
}

void tackleRename(int control_socket, char * str, int *rename_start, char * oldname){
	char request_file[DIR_LENGTH];
	FILE * fp;

	fp = NULL;
	getParameter(request_file, str, 4);
	if(strstr(str, "RNFR") == str){
		if((fp = fopen(request_file, "r")) != NULL && *rename_start == RENAME_FINISH){
			strcpy(oldname, request_file);
			*rename_start = RENAME_START;
			writeMyData("350 The renaming starts.\r\n", control_socket);
			fclose(fp);
		}
		else if(*rename_start != RENAME_FINISH){
			writeMyData("450 You should use RNTO after RNFR.\r\n", control_socket);
			fclose(fp);
		}
		else{
			writeMyData("450 The file doesn't exist.\r\n", control_socket);
		}
	}
	else if(strstr(str, "RNTO") == str){
		if((*rename_start) == RENAME_START){
			if(rename(oldname, request_file) == 0)
			{
				*rename_start = RENAME_FINISH;
				writeMyData("250 The renaming finishes.\r\n", control_socket);
				oldname[0] = '\0';
			}
			else{
				*rename_start = RENAME_FINISH;
				writeMyData("503 Renaming failed.\r\n", control_socket);
				oldname[0] = '\0';
			}
		}
		else{
			writeMyData("503 You should use RNFR after RNTO.\r\n", control_socket);
		}
	}
}

void getParameter(char * request_file, char * str, int control_length){
	char request_file_buf[DIR_LENGTH];

	strcpy(request_file_buf, str + control_length);
	int temp_len = strlen(request_file_buf);
	for(int i = temp_len; i > 0; i--){
		if(iscntrl(request_file_buf[i - 1]) || isspace(request_file_buf[i - 1])){
			request_file_buf[i - 1] = '\0';
		}
		else{
			break;
		}
	}
	int j = 0;
	int flag = 0;
	for(int i = 0; request_file_buf[i] != '\0'; i++){
		if(flag == 0 && (!iscntrl(request_file_buf[i])) && (!isspace(request_file_buf[i]))){
			flag = 1;
		}
		if(flag == 1 && request_file_buf[i] != '\0'){
             request_file[j] = request_file_buf[i]; 
             j++;
		}
		else if(flag == 1){
			break;
		}
	}
	request_file[j] = '\0';
}

void tackleFile(int socket, int control_socket, char * str){
	char current_dir[DIR_LENGTH];
	char request_file[DIR_LENGTH];
	char buffer[BUFFER_LENGTH];
	char file_content[FILE_LENGTH];
	char info[BUFFER_LENGTH];
	char file_info[FILE_INFO_LENGTH];
	FILE* fp;
	int control_length;
	getcwd(current_dir, DIR_LENGTH);
	
	request_file[0] = '\0';
	control_length = strlen(str);
	if(control_length >= 5){
		getParameter(request_file, str, 4);
		snprintf(buffer, DIR_LENGTH, "%s/%s", current_dir, request_file);
	}
	if(strstr(str, "RETR") == str){
		strcpy(info, "150 Opening BINARY mode data connection for ");
		if((fp = fopen(request_file, "rb")) != NULL){
			strcat(info, request_file);
			strcat(info, "\r\n");
		}
		else if((fp = fopen(buffer, "rb")) == NULL){
			printf("Error readfile(): %s(%d)\n", strerror(errno), errno);
			writeMyData("451 The file dosen't exist.\r\n", control_socket);
			return;
		}
		else{
			strcat(info, buffer);
			strcat(info, "\r\n");
		}
		int len;
		writeMyData(info, control_socket);
		while(!feof(fp)){
			len = fread(file_content, sizeof(char), FILE_LENGTH, fp);
			if(send(socket, file_content, len, 0) == -1){
				printf("Error send(): \n");
				writeMyData("426 The client connection is broken or network failure.\r\n", control_socket);
				fclose(fp);
				return;
			}
			bytes_num += len;
		}
		all_file_num += 1;
		writeMyData("226 The file is transfered successfully.\r\n", control_socket);
		fclose(fp);
	}
	else if(strstr(str, "STOR") == str){
		strcpy(info, "150 Opening BINARY mode data connection for ");
		if((fp = fopen(request_file, "wb")) != NULL){
			strcat(info, request_file);
			strcat(info, "\r\n");
			writeMyData(info, control_socket);
		}
		else if((fp = fopen(buffer, "wb")) == NULL){
			strcat(info, buffer);
			strcat(info, "\r\n");
			writeMyData(info, control_socket);
			printf("Error writefile(): %s(%d)\n", strerror(errno), errno);
			writeMyData("451 The file dosen't exist.\r\n", control_socket);
			return;
		}
		else{
			strcat(info, buffer);
			strcat(info, "\r\n");
			writeMyData(info, control_socket);
	    }
		int n = 0; 
		while(1){
			n = recv(socket, file_content, sizeof(file_content), 0);
			if(n > 0){
				if(fwrite(file_content, n, 1, fp) <= 0){
					printf("Error writefile(): %s(%d)\n", strerror(errno), errno);
					writeMyData("530 You can't write to the file.\r\n", control_socket);
					fclose(fp);
					return;
				}
				bytes_num += n;
			}
			else if(n == 0){
				break;
			}
			else if(n < 0){
				writeMyData("426 The client connection is broken or network failure.\r\n", control_socket);
				fclose(fp);
				return;
			}
		}
		all_file_num += 1;
		fclose(fp);
		writeMyData("226 The file is transfered successfully.\r\n", control_socket);
	}
	else if(strstr(str, "LIST") == str || strstr(str, "NLST") == str){

        strcpy(info, "150 Opening ASCII mode data connection for ");
		if(control_length < 5 || strlen(request_file) == 0){
			if(strstr(str, "LIST") == str){
				if(getFileList(current_dir, file_info) == NULL){
					writeMyData("Cannot find the file or directory.\r\n", control_socket);
					return;
				}
				else{
					strcat(info, current_dir);
					strcat(info, "\r\n");
					writeMyData(info, control_socket);
				}
			}
			else{
				if(getShortFileList(current_dir, file_info) == NULL){
					writeMyData("Cannot find the file or directory.\r\n", control_socket);
					return;
				}
				else{
					strcat(info, current_dir);
					strcat(info, "\r\n");
					writeMyData(info, control_socket);
				}
			}
		}
		else{
			if(strstr(str, "LIST") == str){
				if(getFileList(request_file, file_info) == NULL){
					writeMyData("Cannot find the file or directory.\r\n", control_socket);
					return;
				}
				else{
					strcat(info, request_file);
					strcat(info, "\r\n");
					writeMyData(info, control_socket);
				}
			}
			else{
				if(getShortFileList(request_file, file_info) == NULL){
					writeMyData("Cannot find the file or directory.\r\n", control_socket);
					return;
				}
				else{
					strcat(info, request_file);
					strcat(info, "\r\n");
					writeMyData(info, control_socket);
				}
			}
		}
		int len = strlen(file_info);
		if(send(socket, file_info, len, 0) == -1){
				printf("Error send(): \n");
				writeMyData("426 The client connection is broken or network failure.\r\n", control_socket);
				return;
		}
		bytes_num += len;
		writeMyData("226 The list is transfered successfully.\r\n", control_socket);
	}
	else if(strstr(str, "APPE") == str){
		strcpy(info, "150 Opening BINARY mode data connection for ");
		if((fp = fopen(request_file, "ab")) != NULL){
			strcat(info, request_file);
			strcat(info, "\r\n");
			writeMyData(info, control_socket);
		}
		else if((fp = fopen(buffer, "ab")) == NULL){
			strcat(info, buffer);
			strcat(info, "\r\n");
			writeMyData(info, control_socket);
			printf("Error writefile(): %s(%d)\n", strerror(errno), errno);
			writeMyData("451 The file dosen't exist.\r\n", control_socket);
			return;
		}
		else{
			strcat(info, buffer);
			strcat(info, "\r\n");
			writeMyData(info, control_socket);
	    }
		int n = 0; 
		while(1){
			n = recv(socket, file_content, sizeof(file_content), 0);
			if(n > 0){
				if(fwrite(file_content, n, 1, fp) <= 0){
					printf("Error writefile(): %s(%d)\n", strerror(errno), errno);
					writeMyData("530 You can't write to the file.\r\n", control_socket);
					fclose(fp);
					return;
				}
				bytes_num += n;
			}
			else if(n == 0){
				break;
			}
			else if(n < 0){
				writeMyData("426 The client connection is broken or network failure.\r\n", control_socket);
				fclose(fp);
				return;
			}
		}
		all_file_num += 1;
		fclose(fp);
		writeMyData("226 The file is transfered successfully.\r\n", control_socket);
	}
	else{
		tackleCommonCmd(str, control_socket);
	}
}

char* getFileList(char * current_dir, char* file_info){
	DIR *p_dir;
	char list_cmd_info[DIR_LENGTH];
	char list_buf[FILE_LENGTH];
	char file_info_c; //读取详细信息的字符
	FILE *pipe_f;
	list_buf[0] = '\0';

	if((p_dir = opendir(current_dir)) == NULL){
		printf("Cannot open directory %s", current_dir);
		return NULL;
	}
	
	snprintf(list_cmd_info, DIR_LENGTH, "ls -l %s", current_dir);
   	if ((pipe_f = popen(list_cmd_info, "r")) == NULL){
		printf("pipe open error in cmd_list\n");
		return NULL;
    }
    else{
   	    printf("pipe open successfully!, cmd is %s\n", list_cmd_info);
   	}
   	int i = 0;
   	while ((file_info_c = fgetc(pipe_f)) != EOF){
		file_info[i] = file_info_c;
		i++;
    }
    file_info[i - 1] = '\r';
    file_info[i] = '\n';
    file_info[i + 1] = '\0';
   	pclose(pipe_f);
   	return file_info;
}

char* getShortFileList(char * current_dir, char* file_info){  
    struct dirent* ent = NULL;  
    DIR *f_dir;  
    char dir[DIR_LENGTH]; 
    
    file_info[0] = '\0';
    if ((f_dir = opendir(current_dir)) == NULL)  
    {  
        printf("Cannot open directory %s\n", current_dir);  
        return NULL;  
    }  
    while ((ent = readdir(f_dir)) != NULL)  
    {   
        snprintf(dir, DIR_LENGTH, "%s\r\n", ent->d_name);  
        strcat(file_info, dir);
    }
    closedir(f_dir);  
    return file_info;
}

void tackleCommonCmd(char * str, int control_socket){
	if(strstr(str, "QUIT") == str || strstr(str, "ABOR") == str){
		writeMyData("221-Thank you for using the FTP service on ftp.CaoZhangjie.net.\r\n221 Goodbye.\r\n", control_socket);
	}
	else if(strstr(str, "SYST") == str){
		writeMyData("215 UNIX Type: L8\r\n", control_socket);
	}
	else if(strstr(str, "TYPE")){
		if(strstr(str + 4, " I") == str + 4 && strlen(str) <= 8){
			writeMyData("200 Type set to I.\r\n", control_socket);
		}
		else{
			writeMyData("551 Type set to unknown number\r\n", control_socket);
		}
	}
	else if(strstr(str, "CDUP") == str || strstr(str, "CWD") == str || strstr(str, "MKD") == str || strstr(str, "RMD") == str || strstr(str, "DELE") == str){
		tackleChangeDirectory(control_socket, str);
	}
	else if(strstr(str, "PWD") == str){
		tacklePwd(control_socket, str);
	}
	else if(strcmp(str, "NOOP") == 0){
		writeMyData("200 Your NOOP is responded.\r\n", control_socket);
	}
	else if(strstr(str, "NOOP") == str){
		writeMyData("500 NOOP shouldn't have any parameter.\r\n", control_socket);
	}
	else if(strstr(str, "HELP") == str){
		tackleHelp(str, control_socket);
	}
	else if(strstr(str, "MODE") == str){
		changeToUpper(str);
		if(strstr(str, "MODE S") == str){
			writeMyData("200 mode.\r\n", control_socket);
		}
		else{
			writeMyData("504 reject.\r\n", control_socket);
		}
	}
	else if(strstr(str, "STRU") == str){
		changeToUpper(str);
		if(strstr(str, "STRU F") == str){
			writeMyData("200 stru.\r\n", control_socket);
		}
		else{
			writeMyData("504 reject.\r\n", control_socket);
		}
	}
	else if(strstr(str, "ALLO") == str){
		writeMyData("202 accept.\r\n", control_socket);
	}
	else{
		writeMyData("502 The server does not support the verb.\r\n", control_socket);
	}
}

void changeToUpper(char* str){
	int len = strlen(str);
	for(int i = 0; i < len; i++){
		if(islower(str[i])){
			str[i] += 32;
		}
	}
}

void writeMyData(const char * str, int socket){
	int len = strlen(str);
	if(send(socket, str, len, 0) == -1){
		printf("Error write(): %s(%d)\n", strerror(errno), errno);
	}
}

void readMyData(char * str, int socket){
	int p = 0;
	while (1) {
		int n = read(socket, str + p, BUFFER_LENGTH - p);
		if (n < 0) {
			printf("Error read(): %s(%d)\n", strerror(errno), errno);
			break;
		} 
		else if (n == 0 && p == 0) {
			continue;
		}
		else if(p != 0 && n == 0){
			break;
		}
		else {
			p += n;
			if (str[p - 1] == '\n') {
				break;
			}
		}
	}
	str[p - 2] = '\0';
}

void sessionLoop(int connfd, char* sentence, int phase, struct sockaddr_in client){
	int ip_for_client[6];
	int ip_for_server[6];
	char oldname[DIR_LENGTH];
	pthread_t thread;
	int * pasv_socket;
	int rename_state;
	char user_name[USER_INFO_LENGTH];
	char now_ip_server[DIR_LENGTH];
	struct file_thread_arg  f_arg;

	long now_ip_server_n = inet_addr(inet_ntoa(client.sin_addr));
	snprintf(now_ip_server, sizeof(now_ip_server), "%ld,%ld,%ld,%ld,", now_ip_server_n & 0xff, now_ip_server_n >> 8 & 0xff, now_ip_server_n >> 16 & 0xff, now_ip_server_n >> 24 & 0xff);
	pasv_socket = (int *)malloc(sizeof(int));
	rename_state = RENAME_FINISH;
	*pasv_socket = -3;
	welcomeInfo(connfd);
	phase = USER_NAME_PHASE;
	while(1){
		readMyData(sentence, connfd);
		switch(phase){
			case USER_NAME_PHASE:
				if(tackleUser(sentence, connfd, user_name)){
					phase = USER_PASS_PHASE;
				}
				break;
			case USER_PASS_PHASE:
				if(tacklePass(sentence, connfd)){
					phase = USER_LOGIN;
				}
				break;
			case USER_LOGIN:
				if(strstr(sentence, "PORT ") == sentence){
					int number;
					char* ip_address = sentence + 5;
					if(!isdigit(ip_address[0])){
						writeMyData("550 Ip address is illegal.\r\n", connfd);
						break;
					}
					int i = 0;
					char* comma = sentence + 4;
					do{
						++i;
						if(!(number = atoi(comma + 1)) || number > 255 || number < 0){
							if(*(comma + 1) != '0' || *(comma + 2) != ','){
								break;
							}
						}
						ip_for_client[i - 1] = number;
						if(i == 6){
							writeMyData("200 PORT command successful.\r\n", connfd);
							phase = USER_HAS_PORT;
							break;
						}
					}
					while((comma = strchr(comma + 1, ',')) != NULL);
					if(phase != USER_HAS_PORT){
						writeMyData("550 Ip address is illegal.\r\n", connfd);
					}
				}
				else if(strstr(sentence, "PASV") == sentence){
					srand(time(0));
					int number;
					while(1){
						int flag = 1;
						if(!((number = rand() % 65536) > 20000 && number < 65536)){
							continue;
						}
						else{
							for(int i = 0; i < MOST_USER; i++){
								if(number == port_for_pasv[i]){
									flag = 0;
									break;
								}
							}
							if(flag == 1){
								break;
							}
						}
					};
					for(int i = 0; i < MOST_USER; i++){
						if(-1 == port_for_pasv[i]){
							port_for_pasv[i] = number;
							break;
						}
						if(i == MOST_USER - 1){
							writeMyData("425 No TCP connection can be established.\r\n", connfd);
						}
					}

					ip_for_server[4] = number / 256;
					ip_for_server[5] = number % 256;
					char port_str[8];
	   				char addr_str[30];
	   				char message[62];
	   				strcpy(message, "227 Entering Passive Mode (");
					long ip_num = inet_addr(inet_ntoa(client.sin_addr));
					snprintf(addr_str, sizeof(addr_str), "%ld,%ld,%ld,%ld,", ip_num & 0xff, ip_num >> 8 & 0xff, ip_num >> 16 & 0xff, ip_num >> 24 & 0xff);
					snprintf(port_str, sizeof(port_str), "%d,%d", ip_for_server[4], ip_for_server[5]);
					strcat(addr_str, port_str);
					strcat(message, addr_str);
					strcat(message, ")\r\n");
					writeMyData(message, connfd);
					phase = USER_USE_PASV;

					f_arg.in.sin_port = number;
					f_arg.control_socket = connfd;
					f_arg.socket = pasv_socket;
					if(pthread_create(&thread, NULL, handlePasvFileRequest, (void*)&f_arg))
					{
						printf("Error accept(): %s(%d)\n", strerror(errno), errno);
						writeMyData("421-ftp.CaoZhangjie.net is too busy to provide files right now.\r\n421 Please come back in 2040 seconds.\r\n", connfd);
					}
				}
				else if(strstr(sentence, "QUIT") == sentence || strstr(sentence, "ABOR") == sentence){
					writeMyData("221-Thank you for using the FTP service on ftp.CaoZhangjie.net.\r\n221 Goodbye.\r\n", connfd);
					return;
				}
				else if(strstr(sentence, "RNFR") == sentence || strstr(sentence, "RNTO") == sentence){
					tackleRename(connfd, sentence, &rename_state, oldname);
				}
				else if(strstr(sentence, "STAT") == sentence){
					char info[BUFFER_LENGTH];
					snprintf(info, BUFFER_LENGTH, "211-ftp.CaoZhangjie.net FTP server status:\r\nVersion 6.00\r\nConnected to CaoZhangjie.net (%s)\r\nLogged in %s\r\nTYPE: ASCII, FROM: Nonprint; Structure: File; transfer MODE: Stream\r\nNo data connection\r\n", now_ip_server, user_name);
					writeMyData(info, connfd);
					writeMyData("211 End of status", connfd);
				}
				else{
					tackleCommonCmd(sentence, connfd);
				}
				break;
			case USER_HAS_PORT:
				if(strstr(sentence, "RETR") == sentence || strstr(sentence, "STOR") == sentence || strstr(sentence, "LIST") || strstr(sentence, "NLST") == sentence || strstr(sentence, "APPE")){
					f_arg.control_socket = connfd;
					for(int i = 0; i < 6; i++){
						f_arg.ip_num[i] = ip_for_client[i];
					}
					f_arg.sentence = sentence;
					if(pthread_create(&thread, NULL, handlePortFileRequest, (void*)&f_arg))
					{
						printf("Error accept(): %s(%d)\n", strerror(errno), errno);
						writeMyData("421-ftp.CaoZhangjie.net is too busy to provide files right now.\r\n421 Please come back in 2040 seconds.\r\n", connfd);
					}
					phase = USER_LOGIN;
				}
				else if(strstr(sentence, "QUIT") == sentence || strstr(sentence, "ABOR") == sentence){
					writeMyData("221-Thank you for using the FTP service on ftp.CaoZhangjie.net.\r\n221 Goodbye.\r\n", connfd);
					return;
				}
				else if(strstr(sentence, "STAT") == sentence){
					char info[BUFFER_LENGTH];
					snprintf(info, BUFFER_LENGTH, "211-ftp.CaoZhangjie.net FTP server status:\r\nVersion 6.00\r\nConnected to CaoZhangjie.net (%s)\r\nLogged in %s\r\nTYPE: ASCII, FROM: Nonprint; Structure: File; transfer MODE: Stream\r\nNo data connection\r\n", now_ip_server, user_name);
					writeMyData(info, connfd);
					writeMyData("211 End of status", connfd);
				}
				else if(strstr(sentence, "RNFR") == sentence || strstr(sentence, "RNTO") == sentence){
					tackleRename(connfd, sentence, &rename_state, oldname);
				}
				else{
					tackleCommonCmd(sentence, connfd);
				}
				break;
			case USER_USE_PASV:
				if(strstr(sentence, "RETR") == sentence || strstr(sentence, "STOR") == sentence || strstr(sentence, "LIST") || strstr(sentence, "NLST") == sentence || strstr(sentence, "APPE")){
					while(*pasv_socket == -2);
					if(*pasv_socket == -3){
						writeMyData("550 You should use PASV or PORT before file transfering.\r\n", connfd);
					}
					else if(*pasv_socket == -1){
						writeMyData("550 Net error.\r\n", connfd);
					}
					else{
						tackleFile(*pasv_socket, connfd, sentence);
						close(*pasv_socket);
					}
					phase = USER_LOGIN;
					*pasv_socket = -3;
				}
				else if(strstr(sentence, "QUIT") == sentence || strstr(sentence, "ABOR") == sentence){
					writeMyData("221-Thank you for using the FTP service on ftp.CaoZhangjie.net.\r\n221 Goodbye.\r\n", connfd);
					return;
				}
				else if(strstr(sentence, "STAT") == sentence){
					char info[BUFFER_LENGTH];
					snprintf(info, BUFFER_LENGTH, "211-ftp.CaoZhangjie.net FTP server status:\r\nVersion 6.00\r\nConnected to CaoZhangjie.net (%s)\r\nLogged in %s\r\nTYPE: ASCII, FROM: Nonprint; Structure: File; transfer MODE: Stream\r\nNo data connection\r\n", now_ip_server, user_name);
					writeMyData(info, connfd);
					writeMyData("211 End of status", connfd);
				}
				else if(strstr(sentence, "RNFR") == sentence || strstr(sentence, "RNTO") == sentence){
					tackleRename(connfd, sentence, &rename_state, oldname);
				}
				else{
					tackleCommonCmd(sentence, connfd);
				}
				break;
		}
	}
}

void pasvLoop(struct file_thread_arg * in_port){
	*(in_port->socket) = -2;
	int listenfd, connfd;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	unsigned int in_size = sizeof(struct sockaddr_in);

	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(in_port->in.sin_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return;
	}

	if (listen(listenfd, 1) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return;
	}
	while(1){
		if ((connfd = accept(listenfd, (struct sockaddr *)&client, &in_size)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			writeMyData("425 No TCP connection is established.\r\n", in_port->control_socket);
			*(in_port->socket) = -1;
			for(int i = 0; i < MOST_USER; i++){
				if(port_for_pasv[i] == in_port->in.sin_port){
					port_for_pasv[i] = -1;
					break;
				}
			}
		}
		else{
			for(int i = 0; i < MOST_USER; i++){
				if(port_for_pasv[i] == in_port->in.sin_port){
					port_for_pasv[i] = -1;
					break;
				}
			}
			*(in_port->socket) = connfd;
		}
		return;
	}
}

void portLoop(struct file_thread_arg * client){
	int sockfd;
	struct sockaddr_in addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(client->ip_num[4] * 256 + client->ip_num[5]);
	char str_ip[20];
	char buffer[4];
	sprintf(str_ip, "%d", client->ip_num[0]);
	sprintf(buffer, "%d", client->ip_num[1]);
	strcat(str_ip, ".");
	strcat(str_ip, buffer);
	sprintf(buffer, "%d", client->ip_num[2]);
	strcat(str_ip, ".");
	strcat(str_ip, buffer);
	sprintf(buffer, "%d", client->ip_num[3]);
	strcat(str_ip, ".");
	strcat(str_ip, buffer);
	//把ip转换为地址
	if (inet_pton(AF_INET, str_ip, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		writeMyData("425 No TCP connection is established because of invalid ip address.\r\n", client->control_socket);
		return;
	}

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		writeMyData("425 No TCP connection is established.\r\n", client->control_socket);
		return;
	}
	tackleFile(sockfd, client->control_socket, client->sentence);
	close(sockfd);
}
#include "User.h"

FILE *fw = new FILE;
FILE *fr = new FILE;
char SYS_NAME[] = "a.sys";
int inode_bitmap[64]; 
int block_bitmap[10240];
int cur_addr = -1;
SuperBlock *superblock = ((struct SuperBlock*)malloc(sizeof(struct SuperBlock)));


void tOpenDiskFile() {
	strcpy(SYS_NAME, "my.sys");
	OpenDiskFile();
}

void tInitSystem() {
	strcpy(SYS_NAME, "my.sys");
	InitSystem();
	cur_addr = initaddr();
}

void tInstallSystem() {
	strcpy(SYS_NAME, "my.sys");
	InstallSystem();
	cur_addr = superblock->ROOT_ADDR;
}
void tBlockAlloc() {
	int i = BlockAlloc();
}
void tnews() {
	char name[] = { "sys5.sys" };
	news(name);
}
void tsfs() {
	char name[] = { "sys5.sys" };
	sfs(name);
}

void tmkdir() {
	char dirname[] = { "dir1" };
	mkdir(cur_addr, dirname);
}

void tls() {
	ls(cur_addr);
}

void tcd() {
	char dirname[] = { "dir1" };
	cur_addr = cd(cur_addr, dirname);
}

void tcreate(){
	char filename[] = { "file1" };
	create(cur_addr, filename);
}
void twrite() {
	char buffer[] = { "buffer1" };
	char filename[] = { "file1" };
	write(cur_addr,filename, buffer);
}
void topen() {
	char filename[] = { "file1" };
	open(cur_addr, filename);
}
void tdel() {
	char filename[] = { "file1" };
	del(cur_addr, filename);
}
void tcdf() {
	char dirname[] = { ".." };
	cur_addr = cd(cur_addr, dirname);
}
void trmdir() {
	char dirname[] = { "dir1" };
	rmdir(cur_addr,dirname);
}
void CommandError(char inputCommand[]){
	printf("%s command error.\n", inputCommand);
};

//处理输入的命令
void Cmd(char inputLine[])	
{
	char inputCommand[100] = { 0 };
	char inputParameter[100] = { 0 };
	char inputParameter2[10240] = { 0 };
	sscanf(inputLine, "%s%s%s", inputCommand, inputParameter, inputParameter2);
	if (strcmp(inputCommand, "new") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			news(inputParameter);
		}
	}
	else if (strcmp(inputCommand, "sfs") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			sfs(inputParameter);
		}
	}
	else if (strcmp(inputCommand, "ls") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			ls(cur_addr);
		}
		else {
			CommandError(inputCommand);
		}
	}
	else if (strcmp(inputCommand, "exit") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			exit();
		}
		else {
			CommandError(inputCommand);
		}
	}
	else if (strcmp(inputCommand, "cd") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			cur_addr = cd(cur_addr, inputParameter);
		}
	}
	else if (strcmp(inputCommand, "mkdir") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			mkdir(cur_addr, inputParameter);
		}
	}
	else if (strcmp(inputCommand, "rmdir") == 0) {
		if (strcmp(inputParameter, "") == 0 || strcmp(inputParameter, ".") == 0 || strcmp(inputParameter, "..") == 0) {
			CommandError(inputCommand);
		}
		else {
			rmdir(cur_addr, inputParameter);
		}
	}
	else if (strcmp(inputCommand, "create") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			create(cur_addr, inputParameter);
		}
	}
	else if (strcmp(inputCommand, "open") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			open(cur_addr, inputParameter);
		}
	}
	else if (strcmp(inputCommand, "delete") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			del(cur_addr, inputParameter);
		}
	}
	else if (strcmp(inputCommand, "close") == 0) {
		if (strcmp(inputParameter, "") == 0) {
			close();
		}
		else {
			CommandError(inputCommand);
		}
	}
	else if (strcmp(inputCommand, "write") == 0) {
		if (strcmp(inputParameter, "") == 0|| strcmp(inputParameter2, "") == 0) {
			CommandError(inputCommand);
		}
		else {
			write(cur_addr, inputParameter, inputParameter2);
		}
	}
}
int main()
{
	/*
	tnews();
	tsfs();
	tmkdir();
	tls();
	tcd();
	tcreate();
	twrite();
	topen();
	close();
	tls();
	tdel();
	tls();
	tcdf();
	trmdir();
	tls();
	*/
	char inputline[100] = { 0 };
	while (1) {
		gets_s(inputline);
		Cmd(inputline);
	}
}
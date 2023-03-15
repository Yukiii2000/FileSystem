#include "User.h"

// 建立文件系统
void news(char SysName[]) {
	strcpy(SYS_NAME, SysName);
	if (OpenDiskFile()) {
		printf("error : %s FileSystem exited.\n", SysName);
	}
	else {
		InitSystem();
		cur_addr = initaddr();
		printf("Create %s FileSystem Successfully.\n", SysName);
	}
};

//打开文件系统
void sfs(char SysName[]) {
	strcpy(SYS_NAME, SysName);
	if (OpenDiskFile()) {
		InstallSystem();
		cur_addr = superblock->ROOT_ADDR;
	}
	//文件不存在
	else {
		printf("error : %s FileSystem is not exist .Use news to create a new one.\n", SysName);
	}
}

//获取帮助
void help() {
}

//退出打开的文件系统
void exit() {
	fclose(fw);
	fclose(fr);
	system("CLS");
}

// 创建子目录
bool mkdir(int curAddr, char dirname[]) {
	//分配和初始化inode
	int newNodeAddr = InodeAlloc();
	if (newNodeAddr < 0) {
		printf("error : no free inode to create dir");
		return FALSE;
	}

	//读出当前目录索引节点
	Inode cur = { 0 };
	fseek(fr, curAddr, SEEK_SET);
	int k = fread(&cur, sizeof(Inode), 1, fr);
	if (ferror(fr))
		printf("error");
	DirectoryEntry dirlist[16] = { 0 };

	//遍历当前目录找到一个空目录项
	int i = 0;//盘块号
	int j = 0;//目录项号
	int block_id = -1;
	int in_block_id = -1;
	for (i = 0; i < 10; i++) {
		int flag = 0;
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		//每个磁盘块存放16个目录项
		for (j = 0; j < 16; j++) {
			//如果该目录下存在同名目录
			if (strcmp(dirlist[j].DIRNAME, dirname) == 0) {
				Inode tmp = { 0 };
				fseek(fr, dirlist[j].inode_addr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, fr);
				if (tmp.INODE_IF_DIR == 1) {
					printf("error : this directory is exits.");
					return FALSE;
				}
			}
			//找到空目录项退出循环
			else if (strcmp(dirlist[j].DIRNAME, "") == 0){
				block_id = i;
				in_block_id = j;
				flag = 1;
				break;
            }
		}
		if (flag) break;
	}

	//如果没有找到空目录项为当前目录分配新的磁盘块
	if (block_id == -1 || in_block_id == -1) {
		for (i = 0; i < 10; i++) {
			if (cur.INODE_ADDR_TABLE[i] == -1) {
				break;
			}
		}
		int blockadrr = BlockAlloc();
		if (blockadrr < 0) {
			return FALSE;
		}
		cur.INODE_ADDR_TABLE[i] = BlockAlloc();
		block_id = i;
		in_block_id = 0;
		memset(dirlist, 0, sizeof(dirlist));
		fseek(fw, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
		fwrite(dirlist, sizeof(dirlist), 1, fw);
		fflush(fw);
	}

	//建立并初始化子目录索引节点
	Inode newNode;
	newNode.FIRST_INDIRECT = -1;
	newNode.INODE_IF_DIR = 1;
	newNode.INDOE_LAST_MODIFY = time(NULL);
	newNode.INODE_LAST_ACCESS = time(NULL);
	newNode.INDOE_NLINK = 0;
	newNode.INODE_EXEC = 1;
	newNode.INODE_ID = superblock->INODE_NUM-superblock->INODE_FREE_NUM;
	newNode.INODE_READ = 1;
	int newblockAddr = BlockAlloc();
	if (newblockAddr == -1) {
		printf("Block allocation failure." );
		return FALSE;
	}

	//初始化子目录目录项表
	DirectoryEntry dirlist2[16] = { 0 };	
	strcpy(dirlist2[0].DIRNAME, ".");
	strcpy(dirlist2[1].DIRNAME, "..");
	dirlist2[0].inode_addr = newNodeAddr;	//当前目录inode地址
	dirlist2[1].inode_addr = curAddr;	//父目录inode地址
	newNode.INODE_ADDR_TABLE[0] = newblockAddr;
	for (k = 1;k < 10; k++) {
		newNode.INODE_ADDR_TABLE[k] = -1;
	}
	//目录项表写磁盘
	fseek(fw, newblockAddr, SEEK_SET);
	fwrite(dirlist2, sizeof(dirlist2), 1, fw);
	fflush(fw);

	//读出当前目录要加入新目录的块
	fseek(fr, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fread(dirlist, sizeof(dirlist), 1, fr);
	fflush(fr);
	//新目录加入到当前目录
	strcpy(dirlist[in_block_id].DIRNAME, dirname);
	dirlist[in_block_id].inode_addr = newNodeAddr;
	//更新当前目录磁盘块
	fseek(fw, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fwrite(dirlist, sizeof(dirlist), 1, fw);
	fflush(fw);

	//更新当前目录inode
	cur.INDOE_NLINK++;
	fseek(fw, curAddr, SEEK_SET);
	fwrite(&cur, sizeof(Inode), 1, fw);
	fflush(fw);

	//更新新目录inode
	fseek(fw, newNodeAddr, SEEK_SET);
	fwrite(&newNode, sizeof(Inode), 1, fw);
	fflush(fw);

	return TRUE;
}

//删除子目录
void rmdir(int curAddr, char dirname[]) {
    //调用cd得到当前子目录索引节点地址
	int dirNodeAddr = cd(curAddr, dirname);
	Inode son = { 0 };
	fseek(fr, dirNodeAddr, SEEK_SET);
	fread(&son, sizeof(Inode), 1, fr);
	if (son.INODE_IF_DIR != 1) {
		printf("error : %s is not a dir.\n",dirname);
		return;
	}

	//删除子目录和子文件
	DirectoryEntry dirlist[16] = { 0 }; //临时目录表
	int i = 0;//磁盘块号
	int j = 0;//目录项号
	for (i = 0; i < 10; i++) {
		//盘块未分配地址
		if (son.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		//遍历索引块中的目录项
		fseek(fr, son.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		for (j = 0; j < 16; j++) {
			//不能删除本身和父目录
			if ((strcmp(dirlist[j].DIRNAME, "")==0)||(dirlist[j].inode_addr < 0)|| (strcmp(dirlist[j].DIRNAME, "..") == 0)|| (strcmp(dirlist[j].DIRNAME, ".") == 0))
				continue;
			Inode newNode = { 0 };
			fseek(fr, dirlist[j].inode_addr, SEEK_SET);
			fread(&newNode, sizeof(Inode), 1, fr);
			//删除子文件或子目录
			if (newNode.INODE_IF_DIR == 1) {
				rmdir(dirNodeAddr, dirlist[j].DIRNAME);
			}
			else {
				del(dirNodeAddr, dirlist[j].DIRNAME);
			}
		}
	}

    //修改当前目录索引节点
	Inode cur;
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist1[16] = { 0 }; //临时目录表
	//遍历当前节点的索引块
	for (i = 0; i < 10; i++) {
		//盘块未分配地址
		if (cur.INODE_ADDR_TABLE[i] < 0) {
			continue;
		}
		//遍历索引块中的目录项表
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist1, sizeof(dirlist1), 1, fr);
		fflush(fr);
		//找到要删除的目录项
		for (j = 0; j < 16; j++) {
			Inode newNode = { 0 };
			if ((strcmp(dirlist1[j].DIRNAME, dirname) == 0)) {
				//修改目录项
				strcpy(dirlist1[j].DIRNAME, "");
				dirlist1[j].inode_addr = -1;
				//写回磁盘
				fseek(fw, cur.INODE_ADDR_TABLE[i], SEEK_SET);
				fwrite(dirlist1, sizeof(dirlist), 1, fw);
				fflush(fw);
				cur.INDOE_NLINK--;
				fseek(fw, curAddr, SEEK_SET);
				fwrite(&cur, sizeof(Inode), 1, fw);
				fflush(fw);
				return;
			}
		}
	}
	printf("error : rmdir failed.\n");
	return;
}

void ls(int curAddr) {
	//取出当前节点
	Inode cur = { 0 };
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist[16] = { 0 }; //临时目录表
	//遍历当前节点的索引块 10
	int i = 0;
	int j = 0;
	for (i = 0; i < 10; i++) {
		//如果该盘块未分配地址，跳过
		if (cur.INODE_ADDR_TABLE[i] < 0) {
			continue;
		}
		//遍历索引块中的目录项表
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		for (j = 0; j < 16; j++) {
			if ((strcmp(dirlist[j].DIRNAME, "") != 0)) {
				printf("%s\t", dirlist[j].DIRNAME);
			}
		}
	}
	printf("\n");
	return;
}

//更改当前目录
int cd(int curAddr,char dirname[]) {
	Inode cur;
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist[16] = { 0 }; //临时目录表
	//遍历当前节点的索引块 10
	int i = 0;
	int j = 0;
	for (i = 0; i < 10; i++) {
		//如果该盘块未分配地址，跳过
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		//遍历索引块中的dirlist 16
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		for (j = 0; j < 16; j++) {
			Inode newNode = { 0 };
			fseek(fr, dirlist[j].inode_addr, SEEK_SET);
			fread(&newNode, sizeof(Inode), 1, fr);
			if (strcmp(dirlist[j].DIRNAME, dirname) == 0) {
				if (newNode.INODE_IF_DIR != 1) {
					printf("error : %s is not a dirname.\n",dirname);
					return -1;
				}
				return dirlist[j].inode_addr;
			}
		}
	}
	printf("error : no dir named %s .\n", dirname);
	return -1;
}     

bool create(int curAddr,char filename[]) {
	//分配和初始化inode
	int newNodeAddr = InodeAlloc();
	if (newNodeAddr < 0) {
		printf("error : no free inode to create dir");
		return FALSE;
	}
	//插入到当前目录中
	Inode cur = { 0 };
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);

	DirectoryEntry dirlist[16] = { 0 };

	int i = 0;//盘块号
	int j = 0;//目录项号
	int block_id = -1;
	int in_block_id = -1;
	for (i = 0; i < 10; i++) {
		//如果该盘块未用，没有存放目录，跳过
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		int flag = 0;
		//每个磁盘块存放16个目录项，取出
		for (j = 0; j < 16; j++) {
			//如果该目录下存在同名文件
			if (strcmp(dirlist[j].DIRNAME, filename) == 0) {
				Inode tmp = { 0 };
				fseek(fr, dirlist[j].inode_addr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, fr);
				if (tmp.INODE_IF_DIR == 0) {
					printf("error : this file is exits.");
					return FALSE;
				}
			}
			else if (strcmp(dirlist[j].DIRNAME, "") == 0) {
				block_id = i;
				in_block_id = j;
				flag = 1;
				break;
			}
		}
		if (flag) break;
	}
	if (block_id == -1 || in_block_id == -1) {
		for (i = 0; i < 10; i++) {
			if (cur.INODE_ADDR_TABLE[i] == -1) {
				break;
			}
		}
		int blockadrr = BlockAlloc();
		if (blockadrr < 0) {
			return FALSE;
		}
		cur.INODE_ADDR_TABLE[i] = BlockAlloc();
		block_id = i;
		in_block_id = 0;
		memset(dirlist, 0, sizeof(dirlist));
		fseek(fw, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
		fwrite(dirlist, sizeof(dirlist), 1, fw);
		fflush(fw);
	}
	//建立子目录索引节点
	Inode newNode;
	newNode.FIRST_INDIRECT = -1;
	newNode.INODE_IF_DIR = -1;
	newNode.INDOE_LAST_MODIFY = time(NULL);
	newNode.INODE_LAST_ACCESS = time(NULL);
	newNode.INDOE_NLINK = 0;
	newNode.INODE_EXEC = 1;
	newNode.INODE_WRITE = 1;
	newNode.INODE_ID = superblock->INODE_NUM - superblock->INODE_FREE_NUM;
	newNode.INODE_READ = 1;
	for (i = 0; i < 10; i++) {
		newNode.INODE_ADDR_TABLE[i] = -1;
	}
	//取出当前目录要加入新目录的块
	fseek(fr, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fread(dirlist, sizeof(dirlist), 1, fr);
	fflush(fr);
	//新目录加入到当前目录
	strcpy(dirlist[in_block_id].DIRNAME, filename);
	dirlist[in_block_id].inode_addr = newNodeAddr;
	//更新当前目录磁盘块
	fseek(fw, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fwrite(dirlist, sizeof(dirlist), 1, fw);
	fflush(fw);
	//更新当前目录inode
	fseek(fw, curAddr, SEEK_SET);
	fwrite(&cur, sizeof(Inode), 1, fw);
	fflush(fw);
	//更新新目录inode
	cur.INDOE_NLINK++;
	fseek(fw, newNodeAddr, SEEK_SET);
	fwrite(&newNode, sizeof(Inode), 1, fw);
	fflush(fw);

	return TRUE;
}

//打开文件
void open(int curAddr, char filename[]) {
	system("CLS");
	int t = _open(curAddr, filename,0);
	return;
}

//关闭文件
void close(){
	system("CLS");
}

//删除文件
void del(int curAddr, char filename[]) {

	int fileNodeAddr = _open(curAddr, filename, 1);
	Inode son = { 0 };
	fseek(fr, fileNodeAddr, SEEK_SET);
	fread(&son, sizeof(Inode), 1, fr);
	if (son.INODE_IF_DIR == 1) {
		printf("error : %s is not a file.\n", filename);
		return;
	}

	int  i = 0;
	for (i = 0; i < 10; i++) {
		//如果该盘块未分配地址，跳过
		if (son.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		char buffer[512] = { 0 };
		fseek(fw, son.INODE_ADDR_TABLE[i], SEEK_SET);
		fwrite(&son, sizeof(buffer), 1, fw);
		BlockFree(son.INODE_ADDR_TABLE[i]);
	}
	InodeFree(fileNodeAddr);
	Inode cur = { 0 };
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist[16] = { 0 };

	int j = 0;//目录项号
	for (i = 0; i < 10; i++) {
		//如果该盘块未用，没有存放目录，跳过
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		//每个磁盘块存放16个目录项，取出
		for (j = 0; j < 16; j++) {
			//如果该目录下存在同名文件
			if (strcmp(dirlist[j].DIRNAME, filename) == 0) {
				Inode tmp = { 0 };
				fseek(fr, dirlist[j].inode_addr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, fr);
				if (tmp.INODE_IF_DIR == 1) {
					printf("error : this  is not a file.");
					return;
				}
				else {
					char t[] = "";
					strcpy(dirlist[j].DIRNAME, t);
					dirlist[j].inode_addr = -1;
				}
			}
		}
		fseek(fw, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fwrite(dirlist, sizeof(dirlist), 1, fw);
		cur.INDOE_NLINK--;
		fseek(fw, curAddr, SEEK_SET);
		fwrite(&cur, sizeof(Inode), 1, fw);
		return;
	}
	printf("error : del failed");
	return;
}

//写文件
bool write(int curAddr,char filename[],char buf[]) {

	int fileNodeAddr = _open(curAddr, filename, 1);
	Inode fileInode = { 0 };
	fseek(fr, fileNodeAddr, SEEK_SET);
	fread(&fileInode, sizeof(Inode), 1, fr);
	if (fileInode.INODE_WRITE != 1) {
		printf("error : file read only.\n");
	}

	//将buf内容写入磁盘块 
	int k;
	int len = strlen(buf);	//文件长度，单位为字节
	for (k = 0; k < len; k += superblock->BLOCK_SIZE) {	
		int curblockAddr;
		if (fileInode.INODE_ADDR_TABLE[ k / superblock->BLOCK_SIZE] == -1) {
			curblockAddr = BlockAlloc();
			if (curblockAddr == -1) {
				printf("error : block allocation failure.");
				return FALSE;
			}
			fileInode.INODE_ADDR_TABLE[k / superblock->BLOCK_SIZE] = curblockAddr;
		}
		else {
			curblockAddr = fileInode.INODE_ADDR_TABLE[k / superblock->BLOCK_SIZE];
		}
		//写入到当前目录的磁盘块
		fseek(fw, curblockAddr, SEEK_SET);
		int m = fwrite(buf, sizeof(buf), 1, fw);
		fflush(fw);
	}

	//更新该文件大小
	fileInode.INODE_SIZE = len;
	fileInode.INDOE_LAST_MODIFY = time(NULL);
	fseek(fw, fileNodeAddr, SEEK_SET);
	fwrite(&fileInode, sizeof(Inode), 1, fw);
	fflush(fw);
	return TRUE;
}

//打开文件
int _open(int curAddr, char filename[],int mode) {
	//读出当前索引节点
	Inode cur;
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist[16] = { 0 }; //临时目录表

	//遍历当前节点的索引块 10
	int i = 0;
	int j = 0;
	int k = 0;
	for (i = 0; i < 10; i++) {
		//如果该盘块未分配地址，跳过
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		//遍历索引块中的目录项表
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		for (j = 0; j < 16; j++) {
			Inode newNode = { 0 };
			fseek(fr, dirlist[j].inode_addr, SEEK_SET);
			fread(&newNode, sizeof(Inode), 1, fr);
			if ((strcmp(dirlist[j].DIRNAME, filename) == 0)) {
				if (newNode.INODE_IF_DIR == 1) {
					printf("error : %s is  a dirname.\n", filename);
					return -1;
				}
				else {
					//由open调用显示内容
					if (mode == 0) {
						newNode.INODE_LAST_ACCESS = time(NULL);
						for (k = 0; k < 10; k++) {
							if (newNode.INODE_ADDR_TABLE[k] < 0) {
								continue;
							}
							char buffer[512] = { 0 };
							fseek(fr, newNode.INODE_ADDR_TABLE[k], SEEK_SET);
							fread(buffer, sizeof(buffer), 1, fr);
							fflush(fr);
							printf("%s", buffer);
						}
					}
					//返回文件索引节点地址
					return dirlist[j].inode_addr;
				}
			}
		}
	}
	printf("error : no file named %s.\n", filename);
	return -1;
}

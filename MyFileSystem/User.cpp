#include "User.h"

// �����ļ�ϵͳ
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

//���ļ�ϵͳ
void sfs(char SysName[]) {
	strcpy(SYS_NAME, SysName);
	if (OpenDiskFile()) {
		InstallSystem();
		cur_addr = superblock->ROOT_ADDR;
	}
	//�ļ�������
	else {
		printf("error : %s FileSystem is not exist .Use news to create a new one.\n", SysName);
	}
}

//��ȡ����
void help() {
}

//�˳��򿪵��ļ�ϵͳ
void exit() {
	fclose(fw);
	fclose(fr);
	system("CLS");
}

// ������Ŀ¼
bool mkdir(int curAddr, char dirname[]) {
	//����ͳ�ʼ��inode
	int newNodeAddr = InodeAlloc();
	if (newNodeAddr < 0) {
		printf("error : no free inode to create dir");
		return FALSE;
	}

	//������ǰĿ¼�����ڵ�
	Inode cur = { 0 };
	fseek(fr, curAddr, SEEK_SET);
	int k = fread(&cur, sizeof(Inode), 1, fr);
	if (ferror(fr))
		printf("error");
	DirectoryEntry dirlist[16] = { 0 };

	//������ǰĿ¼�ҵ�һ����Ŀ¼��
	int i = 0;//�̿��
	int j = 0;//Ŀ¼���
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
		//ÿ�����̿���16��Ŀ¼��
		for (j = 0; j < 16; j++) {
			//�����Ŀ¼�´���ͬ��Ŀ¼
			if (strcmp(dirlist[j].DIRNAME, dirname) == 0) {
				Inode tmp = { 0 };
				fseek(fr, dirlist[j].inode_addr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, fr);
				if (tmp.INODE_IF_DIR == 1) {
					printf("error : this directory is exits.");
					return FALSE;
				}
			}
			//�ҵ���Ŀ¼���˳�ѭ��
			else if (strcmp(dirlist[j].DIRNAME, "") == 0){
				block_id = i;
				in_block_id = j;
				flag = 1;
				break;
            }
		}
		if (flag) break;
	}

	//���û���ҵ���Ŀ¼��Ϊ��ǰĿ¼�����µĴ��̿�
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

	//��������ʼ����Ŀ¼�����ڵ�
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

	//��ʼ����Ŀ¼Ŀ¼���
	DirectoryEntry dirlist2[16] = { 0 };	
	strcpy(dirlist2[0].DIRNAME, ".");
	strcpy(dirlist2[1].DIRNAME, "..");
	dirlist2[0].inode_addr = newNodeAddr;	//��ǰĿ¼inode��ַ
	dirlist2[1].inode_addr = curAddr;	//��Ŀ¼inode��ַ
	newNode.INODE_ADDR_TABLE[0] = newblockAddr;
	for (k = 1;k < 10; k++) {
		newNode.INODE_ADDR_TABLE[k] = -1;
	}
	//Ŀ¼���д����
	fseek(fw, newblockAddr, SEEK_SET);
	fwrite(dirlist2, sizeof(dirlist2), 1, fw);
	fflush(fw);

	//������ǰĿ¼Ҫ������Ŀ¼�Ŀ�
	fseek(fr, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fread(dirlist, sizeof(dirlist), 1, fr);
	fflush(fr);
	//��Ŀ¼���뵽��ǰĿ¼
	strcpy(dirlist[in_block_id].DIRNAME, dirname);
	dirlist[in_block_id].inode_addr = newNodeAddr;
	//���µ�ǰĿ¼���̿�
	fseek(fw, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fwrite(dirlist, sizeof(dirlist), 1, fw);
	fflush(fw);

	//���µ�ǰĿ¼inode
	cur.INDOE_NLINK++;
	fseek(fw, curAddr, SEEK_SET);
	fwrite(&cur, sizeof(Inode), 1, fw);
	fflush(fw);

	//������Ŀ¼inode
	fseek(fw, newNodeAddr, SEEK_SET);
	fwrite(&newNode, sizeof(Inode), 1, fw);
	fflush(fw);

	return TRUE;
}

//ɾ����Ŀ¼
void rmdir(int curAddr, char dirname[]) {
    //����cd�õ���ǰ��Ŀ¼�����ڵ��ַ
	int dirNodeAddr = cd(curAddr, dirname);
	Inode son = { 0 };
	fseek(fr, dirNodeAddr, SEEK_SET);
	fread(&son, sizeof(Inode), 1, fr);
	if (son.INODE_IF_DIR != 1) {
		printf("error : %s is not a dir.\n",dirname);
		return;
	}

	//ɾ����Ŀ¼�����ļ�
	DirectoryEntry dirlist[16] = { 0 }; //��ʱĿ¼��
	int i = 0;//���̿��
	int j = 0;//Ŀ¼���
	for (i = 0; i < 10; i++) {
		//�̿�δ�����ַ
		if (son.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		//�����������е�Ŀ¼��
		fseek(fr, son.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		for (j = 0; j < 16; j++) {
			//����ɾ������͸�Ŀ¼
			if ((strcmp(dirlist[j].DIRNAME, "")==0)||(dirlist[j].inode_addr < 0)|| (strcmp(dirlist[j].DIRNAME, "..") == 0)|| (strcmp(dirlist[j].DIRNAME, ".") == 0))
				continue;
			Inode newNode = { 0 };
			fseek(fr, dirlist[j].inode_addr, SEEK_SET);
			fread(&newNode, sizeof(Inode), 1, fr);
			//ɾ�����ļ�����Ŀ¼
			if (newNode.INODE_IF_DIR == 1) {
				rmdir(dirNodeAddr, dirlist[j].DIRNAME);
			}
			else {
				del(dirNodeAddr, dirlist[j].DIRNAME);
			}
		}
	}

    //�޸ĵ�ǰĿ¼�����ڵ�
	Inode cur;
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist1[16] = { 0 }; //��ʱĿ¼��
	//������ǰ�ڵ��������
	for (i = 0; i < 10; i++) {
		//�̿�δ�����ַ
		if (cur.INODE_ADDR_TABLE[i] < 0) {
			continue;
		}
		//�����������е�Ŀ¼���
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist1, sizeof(dirlist1), 1, fr);
		fflush(fr);
		//�ҵ�Ҫɾ����Ŀ¼��
		for (j = 0; j < 16; j++) {
			Inode newNode = { 0 };
			if ((strcmp(dirlist1[j].DIRNAME, dirname) == 0)) {
				//�޸�Ŀ¼��
				strcpy(dirlist1[j].DIRNAME, "");
				dirlist1[j].inode_addr = -1;
				//д�ش���
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
	//ȡ����ǰ�ڵ�
	Inode cur = { 0 };
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist[16] = { 0 }; //��ʱĿ¼��
	//������ǰ�ڵ�������� 10
	int i = 0;
	int j = 0;
	for (i = 0; i < 10; i++) {
		//������̿�δ�����ַ������
		if (cur.INODE_ADDR_TABLE[i] < 0) {
			continue;
		}
		//�����������е�Ŀ¼���
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

//���ĵ�ǰĿ¼
int cd(int curAddr,char dirname[]) {
	Inode cur;
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist[16] = { 0 }; //��ʱĿ¼��
	//������ǰ�ڵ�������� 10
	int i = 0;
	int j = 0;
	for (i = 0; i < 10; i++) {
		//������̿�δ�����ַ������
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		//�����������е�dirlist 16
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
	//����ͳ�ʼ��inode
	int newNodeAddr = InodeAlloc();
	if (newNodeAddr < 0) {
		printf("error : no free inode to create dir");
		return FALSE;
	}
	//���뵽��ǰĿ¼��
	Inode cur = { 0 };
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);

	DirectoryEntry dirlist[16] = { 0 };

	int i = 0;//�̿��
	int j = 0;//Ŀ¼���
	int block_id = -1;
	int in_block_id = -1;
	for (i = 0; i < 10; i++) {
		//������̿�δ�ã�û�д��Ŀ¼������
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		int flag = 0;
		//ÿ�����̿���16��Ŀ¼�ȡ��
		for (j = 0; j < 16; j++) {
			//�����Ŀ¼�´���ͬ���ļ�
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
	//������Ŀ¼�����ڵ�
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
	//ȡ����ǰĿ¼Ҫ������Ŀ¼�Ŀ�
	fseek(fr, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fread(dirlist, sizeof(dirlist), 1, fr);
	fflush(fr);
	//��Ŀ¼���뵽��ǰĿ¼
	strcpy(dirlist[in_block_id].DIRNAME, filename);
	dirlist[in_block_id].inode_addr = newNodeAddr;
	//���µ�ǰĿ¼���̿�
	fseek(fw, cur.INODE_ADDR_TABLE[block_id], SEEK_SET);
	fwrite(dirlist, sizeof(dirlist), 1, fw);
	fflush(fw);
	//���µ�ǰĿ¼inode
	fseek(fw, curAddr, SEEK_SET);
	fwrite(&cur, sizeof(Inode), 1, fw);
	fflush(fw);
	//������Ŀ¼inode
	cur.INDOE_NLINK++;
	fseek(fw, newNodeAddr, SEEK_SET);
	fwrite(&newNode, sizeof(Inode), 1, fw);
	fflush(fw);

	return TRUE;
}

//���ļ�
void open(int curAddr, char filename[]) {
	system("CLS");
	int t = _open(curAddr, filename,0);
	return;
}

//�ر��ļ�
void close(){
	system("CLS");
}

//ɾ���ļ�
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
		//������̿�δ�����ַ������
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

	int j = 0;//Ŀ¼���
	for (i = 0; i < 10; i++) {
		//������̿�δ�ã�û�д��Ŀ¼������
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		fseek(fr, cur.INODE_ADDR_TABLE[i], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, fr);
		fflush(fr);
		//ÿ�����̿���16��Ŀ¼�ȡ��
		for (j = 0; j < 16; j++) {
			//�����Ŀ¼�´���ͬ���ļ�
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

//д�ļ�
bool write(int curAddr,char filename[],char buf[]) {

	int fileNodeAddr = _open(curAddr, filename, 1);
	Inode fileInode = { 0 };
	fseek(fr, fileNodeAddr, SEEK_SET);
	fread(&fileInode, sizeof(Inode), 1, fr);
	if (fileInode.INODE_WRITE != 1) {
		printf("error : file read only.\n");
	}

	//��buf����д����̿� 
	int k;
	int len = strlen(buf);	//�ļ����ȣ���λΪ�ֽ�
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
		//д�뵽��ǰĿ¼�Ĵ��̿�
		fseek(fw, curblockAddr, SEEK_SET);
		int m = fwrite(buf, sizeof(buf), 1, fw);
		fflush(fw);
	}

	//���¸��ļ���С
	fileInode.INODE_SIZE = len;
	fileInode.INDOE_LAST_MODIFY = time(NULL);
	fseek(fw, fileNodeAddr, SEEK_SET);
	fwrite(&fileInode, sizeof(Inode), 1, fw);
	fflush(fw);
	return TRUE;
}

//���ļ�
int _open(int curAddr, char filename[],int mode) {
	//������ǰ�����ڵ�
	Inode cur;
	fseek(fr, curAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, fr);
	DirectoryEntry dirlist[16] = { 0 }; //��ʱĿ¼��

	//������ǰ�ڵ�������� 10
	int i = 0;
	int j = 0;
	int k = 0;
	for (i = 0; i < 10; i++) {
		//������̿�δ�����ַ������
		if (cur.INODE_ADDR_TABLE[i] == -1) {
			continue;
		}
		//�����������е�Ŀ¼���
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
					//��open������ʾ����
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
					//�����ļ������ڵ��ַ
					return dirlist[j].inode_addr;
				}
			}
		}
	}
	printf("error : no file named %s.\n", filename);
	return -1;
}

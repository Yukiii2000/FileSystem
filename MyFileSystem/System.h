#pragma once
#include "Global.h"
#define c_BLOCKS_PER_GROUP 64
//������
struct SuperBlock {
	int BLOCK_NUM;                             //���̿���
	int BLOCK_SIZE;                            //���̿��С
	int BLOCKS_PER_GROUP;                      //ÿ�����
	
	int FREE_BLOCK_NUM;                        //���д��̿���
	int FREE_BLOCK_TABLE[c_BLOCKS_PER_GROUP];  //���п��
	int  FREE_BLOCK_ADDR;                      //���п�ָ��

	int INODE_NUM;                             //�����ڵ���
	int INODE_SIZE;                            //�����ڵ��С
	int INDOE_ADDR;                            //�����ڵ���ָ��
	int INODE_FREE_NUM;                        //���������ڵ���
	int INODE_BITMAP_ADDR;                     //�����ڵ�λͼ��ʼλ��

	int ROOT_ADDR;                             //���ڵ�λ��

	int DATA_ADDR;                             //��������ʼλ��
	int DATA_BITMAP_ADDR;                      //������λͼλͼ��ʼλ��
};

//�����ڵ�
struct Inode {
	int INODE_ID;                       //�����ڵ��
	int INODE_IF_DIR;                   //�ļ�����
	int INDOE_NLINK;                    //�ļ�������

	int INODE_READ;                     //�ļ�дȨ��
	int INODE_WRITE;                    //�ļ���Ȩ��
	int INODE_EXEC;                     //�ļ�ִ��Ȩ��

	int INODE_SIZE;                     //�ļ���С
	int INODE_ADDR_TABLE[10];           //10��ֱ������
	int FIRST_INDIRECT;                 //һ���������

	time_t INODE_LAST_ACCESS;           //�ļ�������ʱ��
	time_t INDOE_LAST_MODIFY;           //�ļ�����޸�ʱ��
};

//Ŀ¼��
struct DirectoryEntry {
	char DIRNAME[28];                    //Ŀ¼���ļ���
	int inode_addr;                      //Ŀ¼���Ӧ��inode�ڵ�
};

bool InitSystem();                            //��ʼ��ϵͳ
bool OpenDiskFile();                          //�򿪴����ļ�
bool InstallSystem();                         //��װϵͳ
int BlockAlloc();                             //���̿����
bool BlockFree(int);                          //���̿��ͷ�
int InodeAlloc();                             //�����ڵ����
bool InodeFree(int);                          //�����ڵ����
void SuperBlockUpdate();                      //���³�����
int initaddr();                               //������Ŀ¼

extern char SYS_NAME[100];                    //�ļ�ϵͳ��
extern FILE* fw;                              //��������ļ�дָ��
extern FILE* fr;                              //��������ļ���ָ��
extern SuperBlock* superblock;                //������ָ��
extern int inode_bitmap[64];                  //�����ڵ�λͼָ��
extern int block_bitmap[10240];               //���п�ָ��
extern int cur_addr;                          //ϵͳ��ǰ����Ŀ¼��ַ

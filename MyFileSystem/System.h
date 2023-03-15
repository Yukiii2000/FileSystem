#pragma once
#include "Global.h"
#define c_BLOCKS_PER_GROUP 64
//超级块
struct SuperBlock {
	int BLOCK_NUM;                             //磁盘块数
	int BLOCK_SIZE;                            //磁盘块大小
	int BLOCKS_PER_GROUP;                      //每组块数
	
	int FREE_BLOCK_NUM;                        //空闲磁盘块数
	int FREE_BLOCK_TABLE[c_BLOCKS_PER_GROUP];  //空闲块表
	int  FREE_BLOCK_ADDR;                      //空闲块指针

	int INODE_NUM;                             //索引节点数
	int INODE_SIZE;                            //索引节点大小
	int INDOE_ADDR;                            //索引节点区指针
	int INODE_FREE_NUM;                        //空闲索引节点数
	int INODE_BITMAP_ADDR;                     //索引节点位图起始位置

	int ROOT_ADDR;                             //根节点位置

	int DATA_ADDR;                             //数据区起始位置
	int DATA_BITMAP_ADDR;                      //数据区位图位图起始位置
};

//索引节点
struct Inode {
	int INODE_ID;                       //索引节点号
	int INODE_IF_DIR;                   //文件类型
	int INDOE_NLINK;                    //文件连接数

	int INODE_READ;                     //文件写权限
	int INODE_WRITE;                    //文件读权限
	int INODE_EXEC;                     //文件执行权限

	int INODE_SIZE;                     //文件大小
	int INODE_ADDR_TABLE[10];           //10个直接索引
	int FIRST_INDIRECT;                 //一个间接索引

	time_t INODE_LAST_ACCESS;           //文件最后访问时间
	time_t INDOE_LAST_MODIFY;           //文件最后修改时间
};

//目录项
struct DirectoryEntry {
	char DIRNAME[28];                    //目录或文件名
	int inode_addr;                      //目录项对应的inode节点
};

bool InitSystem();                            //初始化系统
bool OpenDiskFile();                          //打开磁盘文件
bool InstallSystem();                         //安装系统
int BlockAlloc();                             //磁盘块分配
bool BlockFree(int);                          //磁盘块释放
int InodeAlloc();                             //索引节点分配
bool InodeFree(int);                          //索引节点回收
void SuperBlockUpdate();                      //更新超级块
int initaddr();                               //创建根目录

extern char SYS_NAME[100];                    //文件系统名
extern FILE* fw;                              //虚拟磁盘文件写指针
extern FILE* fr;                              //虚拟磁盘文件读指针
extern SuperBlock* superblock;                //超级块指针
extern int inode_bitmap[64];                  //索引节点位图指针
extern int block_bitmap[10240];               //空闲块指针
extern int cur_addr;                          //系统当前所处目录地址

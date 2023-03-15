#include"Global.h"
#include"System.h"

//打开磁盘文件
bool OpenDiskFile() {
	fr = fopen(SYS_NAME, "rb");
	if (fr == NULL) 
		return FALSE;
	else
		return TRUE;
}

//新建系统
bool InitSystem() {
	fw = fopen(SYS_NAME, "wb");
	if (fw == NULL) {
	    printf("Virtual disk file open failed.");
		return FALSE;	
	}
	//内存中初始化超级块
	superblock->FREE_BLOCK_NUM = 10240;                 //空闲磁盘块数
	superblock->BLOCKS_PER_GROUP = c_BLOCKS_PER_GROUP;    //空闲块表

	superblock->INODE_NUM = 64;                      //索引节点数
	superblock->INODE_FREE_NUM = 64;                      //索引节点数
	superblock->INODE_SIZE = 128;                     //索引节点大小
	superblock->INDOE_ADDR = 23 * 512;                   //索引节点区起始位置
	superblock->INODE_BITMAP_ADDR = 1 * 512;              //索引节点位图起始位置

	superblock->BLOCK_NUM = 10240;               //磁盘块数
	superblock->BLOCK_SIZE = 512;               //磁盘块大小
	superblock->DATA_ADDR = 23 * 512 + 64 * 128;     //数据区起始位置=索引起始位置+节点大小*节点数
	superblock->FREE_BLOCK_ADDR = 23 * 512 + 64 * 128;
	superblock->DATA_BITMAP_ADDR = 3 * 512;
	//初始化索引节点位图
	memset(inode_bitmap, 0, sizeof(inode_bitmap));
	//初始化盘块位图
	memset(block_bitmap, 0, sizeof(block_bitmap));
	//初始化索引节点
	//initnode();
	//初始化空闲块索引表
	int i, j;
	for (i = superblock->BLOCK_NUM / superblock->BLOCKS_PER_GROUP - 1; i >= 0; i--) {	
		if (i == superblock->BLOCK_NUM / superblock->BLOCKS_PER_GROUP - 1)
			superblock->FREE_BLOCK_TABLE[0] = -1;
		//指向下一个空闲块表
		else
			superblock->FREE_BLOCK_TABLE[0] = superblock->DATA_ADDR + (i + 1) * superblock->BLOCKS_PER_GROUP * superblock->BLOCK_SIZE;	
		for (j = 1; j < superblock->BLOCKS_PER_GROUP; j++) {
			//第i组第j块
			superblock->FREE_BLOCK_TABLE[j] = superblock->DATA_ADDR + (i * superblock->BLOCKS_PER_GROUP + j) * superblock->BLOCK_SIZE;
		}
		//每组的第一块是空闲块索引表
		fseek(fw, superblock->DATA_ADDR + i * superblock->BLOCKS_PER_GROUP * superblock->BLOCK_SIZE, SEEK_SET);
		fwrite(superblock->FREE_BLOCK_TABLE, sizeof(superblock->FREE_BLOCK_TABLE), 1, fw);
	}
	//超级块写入到虚拟磁盘文件
	fseek(fw, 0 , SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, fw);
	//索引节点位图写入虚拟磁盘文件
	fseek(fw, superblock->INODE_BITMAP_ADDR, SEEK_SET);
	fwrite(inode_bitmap, sizeof(inode_bitmap), 1, fw);
	//数据区位图写入虚拟磁盘文件
	fseek(fw, superblock->DATA_BITMAP_ADDR, SEEK_SET);
	fwrite(block_bitmap, sizeof(block_bitmap), 1, fw);
	fflush(fw);

	if (InstallSystem())
		return TRUE;
	else
		return FALSE;
};

//安装已存在的文件系统
bool InstallSystem() {
	fr = fopen(SYS_NAME, "rb");
	fseek(fr,0 ,SEEK_SET);
	int k = fread(superblock, sizeof(SuperBlock), 1, fr);

	fseek(fr, superblock->INODE_BITMAP_ADDR, SEEK_SET);
	fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);

	fseek(fr, superblock->DATA_BITMAP_ADDR, SEEK_SET);
	fread(block_bitmap, sizeof(block_bitmap), 1, fr);
	//fw = fopen(SYS_NAME, "wb+");
	return TRUE;
}

//初始化根目录
int initaddr()
{
	//申请磁盘块
	int blockAddr = BlockAlloc();
	if (blockAddr < 0) {
		printf("error : initial root failed.\n");
		return -1;
	}
	//申请索引节点
	int inodeAddr = InodeAlloc();
	if (blockAddr < 0) {
		printf("error : initial root failed.\n");
		return -1;
	}
	//根目录索引节点
	Inode root;
	root.INDOE_LAST_MODIFY = time(NULL);
	root.INODE_LAST_ACCESS = time(NULL);
	root.INODE_EXEC = 0;
	root.INDOE_NLINK = 1;
	root.INODE_ID = 0;
	root.INODE_READ = 1;
	root.INODE_SIZE = superblock->BLOCK_SIZE;
	root.INODE_IF_DIR = 1;
	root.INODE_WRITE = 1;
	int i;
	root.INODE_ADDR_TABLE[0] = blockAddr;
	for (i = 1; i < sizeof(root.INODE_ADDR_TABLE) / sizeof(int); i++) {
		root.INODE_ADDR_TABLE[i] = -1;
	}
	//创建目录表
	DirectoryEntry dirlist[16] = { 0 };
	strcpy(dirlist[0].DIRNAME, ".");
	dirlist[0].inode_addr = inodeAddr;//自己
	//写磁盘
	fseek(fw, blockAddr, SEEK_SET);
	fwrite(dirlist, sizeof(DirectoryEntry), 1, fw);
	fflush(fw);
	//更新索引节点
	fseek(fw, inodeAddr, SEEK_SET);
	int k = fwrite(&root, sizeof(Inode), 1, fw);
	fflush(fw);
	//更新超级块
	superblock->ROOT_ADDR = inodeAddr;
	SuperBlockUpdate();
	return inodeAddr;
}

void SuperBlockUpdate() {
	fseek(fw, 0, SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, fw);
	fflush(fw);
}

//磁盘块分配（一块）
int BlockAlloc() {
	//没有空闲磁盘块
	if (superblock->FREE_BLOCK_NUM <= 0) {
		printf("error: no free blocks.\n");
		return -1;
	}

	int block;
	int blockAddr;
	block = (superblock->FREE_BLOCK_NUM - 1) % superblock->BLOCKS_PER_GROUP;
	blockAddr = superblock->FREE_BLOCK_TABLE[block];

	//本组空闲块用完
	if (block == 0) {
		superblock->FREE_BLOCK_ADDR = superblock->FREE_BLOCK_TABLE[0];
		fseek(fr, superblock->FREE_BLOCK_ADDR, SEEK_SET);
		fread(superblock->FREE_BLOCK_TABLE, sizeof(superblock->FREE_BLOCK_TABLE), 1, fr);
		fflush(fr);
		superblock->FREE_BLOCK_NUM--;
	}

	//本组空闲块未用完
	else {		
		superblock->FREE_BLOCK_TABLE[block] = -1;	
		block--;		
		superblock->FREE_BLOCK_NUM--;	
	}

	//更新磁盘超级块
	SuperBlockUpdate();

	//更新block位图
	block_bitmap[(blockAddr - superblock->DATA_ADDR) / superblock->BLOCK_SIZE] = 1;
	fseek(fw, (blockAddr - superblock->DATA_ADDR) / superblock->BLOCK_SIZE + superblock->DATA_BITMAP_ADDR, SEEK_SET);
	fwrite(&block_bitmap[(blockAddr - superblock->DATA_ADDR) / superblock->BLOCK_SIZE], sizeof(bool), 1, fw);
	fflush(fw);

	return blockAddr;
}
//磁盘块回收
bool BlockFree(int addr) {
	//判断地址是否为磁盘块起始地址
	if ((addr - superblock->DATA_ADDR) % superblock->BLOCK_SIZE != 0)
	{
		printf("eroor : trying to free a non-block_start_adrress.\n");
		return FALSE;
	}
	int block_id = (addr - superblock->DATA_ADDR) / superblock->BLOCK_SIZE;
	//判断磁盘块是否已使用
	if (block_bitmap[block_id] == 0) {
		printf("error : trying to free a free block.\n");
		return FALSE;
	}
	
	//清空block内容
	char tmp[512] = { 0 };
	fseek(fw, addr, SEEK_SET);
	fwrite(tmp, superblock->BLOCK_SIZE, 1, fw);

	//判断空闲盘块表指针
	int block_gid = (superblock->FREE_BLOCK_NUM - 1) % superblock->BLOCKS_PER_GROUP;
	if(block_gid == superblock->BLOCKS_PER_GROUP - 1){
		//将该空闲块作为新的空闲块堆栈
		superblock->FREE_BLOCK_TABLE[0] = superblock->FREE_BLOCK_ADDR; 
		int i;
		for (i = 1; i < superblock->BLOCKS_PER_GROUP; i++) {
			superblock->FREE_BLOCK_TABLE[i] = -1;	//清空栈元素的其它地址
		}
		fseek(fw, addr, SEEK_SET);
		fwrite(superblock->FREE_BLOCK_TABLE, sizeof(superblock->FREE_BLOCK_TABLE), 1, fw);
	}
	else {
		block_gid++;
		superblock->FREE_BLOCK_TABLE[block_gid] = addr;
	}
	
	//更新超级块
	superblock->FREE_BLOCK_NUM++;
	SuperBlockUpdate();

	//更新数据区位图
	block_bitmap[block_id] = 0;
	//写磁盘
	fseek(fw, superblock->DATA_BITMAP_ADDR + block_id, SEEK_SET);
	fwrite(&block_bitmap[block_id], sizeof(block_bitmap[block_id]), 1, fw);
	fflush(fw);

	return true;
}               

//分配索引节点
int InodeAlloc() {
	if (superblock->INODE_FREE_NUM <= 0){
		printf("error : no free inode.\n");
		return -1;
	}
	//根据索引节点位图查找
	int i = 0;
	int free_inode = -1;
	for (i = 0; i < superblock->INODE_NUM; i++) {
		if (inode_bitmap[i] == 0){
			free_inode = i;
			break;
		}		
	}
	//更新内存superblock
	superblock->INODE_FREE_NUM--;
	//更新磁盘superblock
	SuperBlockUpdate();
	//更新内存inode_bitmap
	inode_bitmap[i] = 1;
	//更新磁盘inode_bitmap
	fseek(fw, superblock->INODE_BITMAP_ADDR + i, SEEK_SET);
	fwrite(&inode_bitmap[i], sizeof(bool), 1, fw);
	fflush(fw);
	//返回索引节点地址
	return superblock->INDOE_ADDR + (free_inode * superblock->INODE_SIZE);
}

//索引节点回收
bool InodeFree(int addr) {
	if (superblock->INODE_FREE_NUM == superblock->INODE_NUM) {
		printf("error : no using inode.\n");
		return FALSE;
	}
	int i = (addr - superblock->INDOE_ADDR) / superblock->INODE_SIZE;
	//更新内存superblock
	superblock->INODE_FREE_NUM++;
	//更新磁盘superblock
	SuperBlockUpdate();
	//更新内存inode_bitmap
	inode_bitmap[i] = 1;
	//更新磁盘inode_bitmap
	fseek(fw, superblock->INODE_BITMAP_ADDR + i, SEEK_SET);
	fwrite(&inode_bitmap[i], sizeof(bool), 1, fw);
	fflush(fw);
	return TRUE;
}
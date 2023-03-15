#pragma once
#include"Global.h"
#include "System.h"

void news(char[]);            //建立文件系统
void sfs(char[]);             //打开文件系统
void exit();                  //退出打开的文件系统
void help();                  //获取帮助
bool mkdir(int,char []);      //创建子目录
void rmdir(int,char[]);       //删除子目录
void ls(int);                 //显示目录
int cd(int,char[]);           //更改当前目录
bool create(int,char[]);      //创建文件
void open(int,char[]);        //打开文件
void close();                 //关闭文件
bool write(int,char[],char[]);//写文件
void del(int,char[]);         //删除文件
int _open(int, char[],int);   //打开文件子调用

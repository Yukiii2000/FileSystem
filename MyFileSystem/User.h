#pragma once
#include"Global.h"
#include "System.h"

void news(char[]);            //�����ļ�ϵͳ
void sfs(char[]);             //���ļ�ϵͳ
void exit();                  //�˳��򿪵��ļ�ϵͳ
void help();                  //��ȡ����
bool mkdir(int,char []);      //������Ŀ¼
void rmdir(int,char[]);       //ɾ����Ŀ¼
void ls(int);                 //��ʾĿ¼
int cd(int,char[]);           //���ĵ�ǰĿ¼
bool create(int,char[]);      //�����ļ�
void open(int,char[]);        //���ļ�
void close();                 //�ر��ļ�
bool write(int,char[],char[]);//д�ļ�
void del(int,char[]);         //ɾ���ļ�
int _open(int, char[],int);   //���ļ��ӵ���

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

typedef int SLNDataType;
typedef struct SListNode
{
	SLNDataType data;//有效数据
	struct SListNode* next;//指向下一结点
}SLN;

SLN* createplist();//创建头结点
SLN* BuySListNode(SLNDataType x);//创建结点
void SLNPrint(SLN* phead);//打印
void SLNPushFront(SLN* phead, SLNDataType x);//头插
void SLNPopFront(SLN* phead);//头删
void SLNPushBack(SLN* phead,SLNDataType x);//尾插
void SLNPopBack(SLN* phead);//尾删
void SLNFind(SLN* phead, SLNDataType x);//查找指定数据，并返回坐标位置
void SLNDestory(SLN* phead);//销毁
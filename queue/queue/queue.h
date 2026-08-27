#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>

typedef int QDataType;

typedef struct LinkNode//结点
{
    QDataType data;
    struct LinkNode* next;
}QNode;

typedef struct LinkQueue//队列
{
    QNode* front;//指向头节点
    QNode* rear;//执行尾结点
    int size;//元素个数
}LinkQueue;


void QueueInit(LinkQueue* q);//初始化
void QueueDestroy(LinkQueue* q);//销毁
bool QueueEmpty(LinkQueue* q);//判空
void QueueSize(LinkQueue* q);//获取元素个数
void QueueFront(LinkQueue* q);//获取队头元素
void QueuePush(LinkQueue* q, QDataType x);//入队
void DeQueue(LinkQueue* q);//出队
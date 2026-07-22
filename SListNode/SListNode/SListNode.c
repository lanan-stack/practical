#include "SListNode.h"

SLN* createplist()//创建头结点
{
	SLN* head = (SLN*)malloc(sizeof(SLN));
	if (head == NULL)
	{
		perror("malloc failed");
	}
	head->next = NULL;
	return head;
}

SLN* BuySListNode(SLNDataType x)//创建结点
{
	SLN* newnode = (SLN*)malloc(sizeof(SLN));
	newnode->data = x;
	newnode->next = NULL;
	
	return newnode;
}

void SLNPrint(SLN* phead)//打印
{
	assert(phead);
	SLN* pcur = phead->next;
	while (pcur) {
		printf("%d->", pcur->data);
		pcur = pcur->next;
	}
	printf("NULL\n");
}

void SLNPushFront(SLN* phead, SLNDataType x)//头插
{
	assert(phead);
	SLN* newnode = BuySListNode(x);
	newnode->next = phead->next;
	phead->next = newnode;
}

void SLNPopFront(SLN* phead)//头删
{
	assert(phead);
	if (phead->next == NULL)
	{
		printf("此链表为空链表");
		return;
	}
	SLN* del = phead->next;
	phead->next = del->next;
	free(del);

}

void SLNPushBack(SLN* phead,SLNDataType x)//尾插
{
	assert(phead);
	SLN* newnode = BuySListNode(x);
	SLN* pcur = phead;
	while (pcur->next !=NULL)
	{
		pcur = pcur->next;
	}
	pcur->next = newnode;
}

void SLNPopBack(SLN* phead)//尾删
{
	assert(phead);
	if (phead->next == NULL)
	{
		printf("空链表");
		return;
	}
	SLN* pcur = phead;
	while (pcur->next->next != NULL)
	{
		pcur = pcur->next;
	}
	free(pcur->next);
	pcur->next = NULL;
}

void SLNFind(SLN* phead, SLNDataType x)//查找
{
	int count = 0;
	assert(phead);
	SLN* pcur = phead;
	while (pcur != NULL) {
		if (pcur->data == x) {
			printf("找到了,该数据在第%d个结点上",count);
			return;
		}
		pcur = pcur->next;
		count++;
	}
	printf("该数据不在这个单链表中");
	return;
}

void SLNDestory(SLN* phead)//销毁
{  
	assert(phead);
	SLN* pcur = phead->next;
	SLN* p = pcur;
	while(pcur != NULL)
	{
		
		pcur = pcur->next;
		free(p);
		p = pcur;
	}
	free(phead);
}
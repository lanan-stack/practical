#include"DCList.h"

void DCListInit1(DCListNode** DCL)//初始化1
{
	
	*DCL = (DCListNode*)malloc(sizeof(DCListNode));
	if (*DCL == NULL)
	{
		perror("malloc failed");
		return;
	}
		(*DCL)->prev = *DCL;
		(*DCL)->next = *DCL;
		(*DCL)->data = 0;
		(*DCL)->list_length = 0;
}

//DCListNode* DCListInit()//初始化2
//{
//	DCListNode* DCL = (DCListNode*)malloc(sizeof(DCListNode));
//	if (DCL == NULL)
//	{
//		perror("malloc fail");
//		return;
//	}
//	DCL->prev = DCL;
//	DCL->next = DCL;
//	DCL->data = 0;
//	return DCL;
//}

void DCListDestory(DCListNode** DCL)//销毁
{
	assert(*DCL);
	DCListNode* cur = (*DCL)->next;
	while (cur != *DCL)
	{
		cur = cur->next;
		free(cur->prev);
	}
	free(*DCL);
	*DCL = NULL;
}

void DCLPrint(DCListNode* DCL)//打印
{
	assert(DCL);
	DCListNode* cur = DCL->next;
	printf("头节点->");
	while (cur != DCL)
	{
		
		printf("%d->", cur->data);
		cur = cur->next;
	}
	printf("头节点\n");
	
}

void DCLPushFront(DCListNode* DCL, DCLDataType x)//头插
{
	DCListNode* dcl = (DCListNode*)malloc(sizeof(DCListNode));
	if (dcl == NULL)
	{
		printf("申请失败");
		perror("malloc fail");
		return;
	}
	dcl->data = x;
	dcl->next = DCL->next;
	DCL->next->prev = dcl;
	DCL->next = dcl;
	dcl->prev = DCL;
	DCL->list_length++;
}

void DCLPushBack(DCListNode* DCL, DCLDataType x)//尾插
{
	DCListNode* dcl = (DCListNode*)malloc(sizeof(DCListNode));
	if (dcl == NULL)
	{
		printf("申请失败");
		perror("malloc fail");
		return;
	}
	dcl->data = x;
	dcl->prev = DCL->prev;
	DCL->prev->next = dcl;
	DCL->prev = dcl;
	dcl->next = DCL;
	DCL->list_length++;
}
void DCLPopFront(DCListNode* DCL)//头删
{
	if (DCL->next == DCL)
	{
		printf("该双向链表为空");
		return;
	}
	DCListNode* del = DCL->next;//第一个节点
	DCListNode* cur = del->next;//第二个节点
	DCL->next = cur;
	cur->prev = DCL;
	free(del);
	DCL->list_length--;
}

void DCLPopBack(DCListNode* DCL)//尾删
{
	if (DCL->prev == DCL)
	{
		printf("该链表为空");
		return;
	}
	DCListNode* del = DCL->prev;//删的节点
	DCListNode* cur = del->prev;
	DCL->prev = cur;
	cur->next = DCL;
	free(del);
	DCL->list_length--;
}

void DCLGetElement(DCListNode* DCL, int i)//获取指定位置节点的元素
{
	assert(DCL);
	if (i > DCL->list_length)
	{
		printf("超过链表长度");
		return;
	}
	if (i < 0)
	{
		printf("请求错误");
		return;
	}
	DCListNode* cur = DCL;
	for (int j = 0; j < i; j++)
	{
		cur = cur->next;
	}
	printf("该位置的数据为%d", cur->data);
}

void DCLInsert(DCListNode* DCL, int i, DCLDataType x)//在第i个节点后插入
{
	assert(DCL);
	if (i<0 || i>DCL->list_length)
	{
		printf("输入错误");
		return;
	}
	DCListNode* dcl = (DCListNode*)malloc(sizeof(DCListNode));
	if (dcl == NULL)
	{
		printf("打印错误");
		perror("malloc fail");
		return;
	}
	dcl->data = x;
	DCListNode* cur = DCL;
	for(int j = 0;j<i;j++)
	{
		cur = cur->next;
	}
	dcl->next = cur->next;
	cur->next->prev = dcl;
	cur->next = dcl;
	dcl->prev = cur;
	DCL->list_length++;
}

void DCLDelete(DCListNode* DCL,int i)//删除指定位置的节点
{
	assert(DCL);
	if (i<0 || i>DCL->list_length)
	{
		printf("请求错误");
		return;
	}
	DCListNode* del = DCL;
	for (int j = 0; j < i; j++)
	{
		del = del->next;
	}
	del->prev = del->next;
	del->next = del->prev;
	free(del);
	DCL->list_length--;
}
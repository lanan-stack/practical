#include"queue.h"

void QueueInit(LinkQueue* q)//初始化
{
	assert(q);
	q->front = q->rear = (QNode*)malloc(sizeof(QNode));
	if (q->front == NULL)
	{
		printf("申请失败");
		perror("malloc failed");
		return;
	}
	//申请成功
	q->front->next = NULL;
	q->size = 0;
}

void QueueDestroy(LinkQueue* q)//销毁
{
	assert(q);
	QNode* cur = q->front;
	while (cur)
	{
		QNode* next = cur->next;
		free(cur);
		cur = next;
	}
	q->front = q->rear = NULL;
	q->size = 0;
}

bool QueueEmpty(LinkQueue* q)//判空
{
	assert(q);
	return (q->size == 0);//空为1，非空为0
}

void QueueSize(LinkQueue* q)//获取元素个数
{
	assert(q);
	printf("队列元素个数为：%d", q->size);
}

void QueueFront(LinkQueue* q)//获取队头元素
{
	assert(q);
	assert(!QueueEmpty(q));
	printf("队头元素为：%d\n", q->front->next->data);
}

void QueuePush(LinkQueue* q, QDataType x)//入队
{
	assert(q);
	QNode* newnode = (QNode*)malloc(sizeof(QNode));
	if (newnode == NULL)
	{
		printf("申请失败");
		perror("malloc failed");
		return;
	}
	newnode->data = x;
	newnode->next = NULL;
	q->rear->next = newnode;
	q->rear = newnode;
	q->size++;
}

void DeQueue(LinkQueue* q)//出队
{
	assert(q);
	assert(!QueueEmpty(q));
	QNode* de = q->front->next;//第一个结点
	printf("队头为：%d", de->data);
	if(de->next)
	{
		q->front->next = de->next;
	}
	else {
		q->front = q->rear = NULL;
	}
	free(de);
	de = NULL;
	q->size--;


}
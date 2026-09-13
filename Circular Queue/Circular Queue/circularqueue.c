#include"circularqueue.h"

circular_queue* circular_queue_create(int k)//初始化1,K为队列长度
{
	circular_queue* obj = (circular_queue*)malloc(sizeof(circular_queue));
	if (obj == NULL)
	{
		printf("申请失败");
		perror("malloc failed");
		exit(-1);
	}
	obj->a = (int*)malloc(sizeof(int) * (k + 1));//牺牲一个空间解决队列空满误判
	obj->front = 0;
	obj->rear = 0;
	obj->N = (k + 1);
	return obj;
}

void circular_queue_destroy(circular_queue* obj)//销毁
{
	assert(obj);
	free(obj->a);
	free(obj);
}

bool circular_queue_isfull(circular_queue* obj)//判满
{
	assert(obj);
	return (obj->rear + 1) % obj->N == obj->front;//1为满，0为非满
}

bool circular_queue_isempty(circular_queue* obj)//判空
{
	assert(obj);
	return obj->front == obj->rear;//1为空，0为非空
}

bool circular_queue_push(circular_queue* obj, int x)//入队
{
	assert(obj);
	if (circular_queue_isfull(obj))
		return false;
	//可以入队
	obj->a[obj->rear] = x;
	obj->rear = (obj->rear + 1) % obj->N;
	return true;//入队成功
}

bool circular_queue_pop(circular_queue* obj)//出队
{
	if (circular_queue_isempty(obj))
		return false;
	//可以出队
	obj->front++;
	obj->front %= obj->N;
	return true;//出队成功
}

int circular_queue_front(circular_queue* obj) //取队头
{
	if (circular_queue_isempty(obj))
		return -1;
	else
		return obj->a[obj->front];
}
int circular_queue_rear(circular_queue* obj) //取队尾
{
	if (circular_queue_isempty(obj))
		return -1;
	else {
		return obj->a[(obj->rear - 1 + obj->N) % obj->N];
	}
}
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>

typedef int CQDataType;

typedef struct
{
	int* a;
	int front;
	int rear;
	int N;//数组空间
}circular_queue;

circular_queue* circular_queue_create(int k);//初始化
void circular_queue_destroy(circular_queue* obj);//销毁
bool circular_queue_isfull(circular_queue* obj);//判满
bool circular_queue_isempty(circular_queue* obj);//判空
bool circular_queue_push(circular_queue* obj, int x);//入队
bool circular_queue_pop(circular_queue* obj);//出队
int circular_queue_front(circular_queue* obj); //取队头
int circular_queue_rear(circular_queue* obj);//取队尾
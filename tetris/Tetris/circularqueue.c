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
// ============================================================
// 以下三个函数是 Phase 4 为"袋子系统"新增的能力（原有函数一字未改）
// ============================================================

// 取当前元素个数
// 原理：rear 是"下一个可写入位置"，front 是"队头位置"，两者之差就是元素个数。
//       因为下标会绕回，所以先加 N 再取模，否则相减可能出现负数。
int circular_queue_size(const circular_queue* obj)
{
	assert(obj);
	return (obj->rear - obj->front + obj->N) % obj->N;
}

// 查看从队头数第 index 个元素（index = 0 表示队头）
// 边界：index 小于 0 或大于等于元素个数时返回 false，队列内容不受影响。
bool circular_queue_peek_at(const circular_queue* obj, int index, int* outValue)
{
	assert(obj);
	if (outValue == NULL)
		return false;
	if (index < 0 || index >= circular_queue_size(obj))
		return false;

	*outValue = obj->a[(obj->front + index) % obj->N];
	return true;
}

// 出队并把被移走的元素值写入 *outValue
// 与原有的 circular_queue_pop 的区别：这个会把值带回来。
// 实现上就是"先取值，再移动队头"，两步都做完才算成功。
bool circular_queue_pop_value(circular_queue* obj, int* outValue)
{
	assert(obj);
	if (outValue == NULL)
		return false;
	if (circular_queue_isempty(obj))
		return false;

	*outValue = obj->a[obj->front];
	obj->front = (obj->front + 1) % obj->N;
	return true;
}

#include"queue.h"

void test()
{
	LinkQueue* q = (LinkQueue*)malloc(sizeof(LinkQueue));
	QueueInit(q);
	QueuePush(q, 1);
	QueuePush(q, 2);
	QueuePush(q, 3);
	QueuePush(q, 4);

	QueueDestroy(q);
}

int main()
{
	test();
	return 0;
}
#include"heap.h"

void testheap()
{
	/*int x = 3;
	int y = 4;
	printf("交换前：%d %d\n", x, y);
	swap(&x, &y);
	printf("交换后：%d %d", x, y);*/
	Heap* hp = (Heap*)malloc(sizeof(Heap));//分配Heap结构体大小的内存
	HeapInit(hp);//初始化
	HeapPush(hp, 3);
	HeapPush(hp, 1);
	HeapPush(hp, 9);
	HeapPush(hp, 2);
	HeapPush(hp, 6);
	HeapPrint(hp);
	HeapPop(hp);
	HeapPrint(hp);
	HeapPop(hp);
	HeapPrint(hp);
	HeapPop(hp);
	HeapPrint(hp);
	HeapPop(hp);
	HeapDestroy(hp);
}
int main()
{
	
	testheap();
	return 0;
}
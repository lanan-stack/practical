#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include<string.h>
#include<stdlib.h>
//数组中有：1 2 3 4 5 1 2 3 4，只有5出现一次，其他数字都出现2次，找出5

int findsingalcount(int* arr,int sz)
{

}

int main()
{
	printf("请输入你所要输入的元素个数：");
		int num = 0;
		scanf("%d\n", &num);
	int* ptr = NULL;
	ptr = (int*)malloc(num * sizeof(int));
	if (ptr == NULL)
	{
		perror("malloc");
		return 0;
	}
	printf("请输入元素：");
	for (int i = 0; i < num; i++)
	{
		scanf("%d", *(ptr + i));
	}
}

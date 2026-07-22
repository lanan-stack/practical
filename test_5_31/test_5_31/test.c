#define _CRT_SECURE_NO_WARNINGS 1
#include<stdio.h>
#include<stdlib.h>
//数组中有：1 2 3 4 5 1 2 3 4，只有5出现一次，其他数字都出现2次，找出5
//打印整数二进制的奇数位和偶数位



int main()
{
	int a = 0;
	printf("请输入要打印的整数：");
	scanf("%d", &a);
	printf("偶数位：");
	for (int i = 30; i >= 0; i -= 2)
	{
		if (a & (1 << i))
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
	}
	printf("\n");
	printf("奇数位：");
	for (int i = 31; i >= 1; i -= 2)
	{
		if (a & (1 << i))
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
	}
	return 0;
}
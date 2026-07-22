#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include<stdlib.h>

int bin_search(int arr[], int left, int right, int key)
{
	while (left <= right)
	{
		if (arr[(left + right) / 2] < key)
		{
			left = (left + right) / 2 + 1;
		}
		else if (arr[(left + right) / 2] > key)
		{
			right = (left + right) / 2 + 1;
		}
		else
		{
			printf("%d", arr[(left + right) / 2]);
		}
		return ;
}
int main()
{
	int r = 0;
	int n = 0;
	int key = 0;
	printf("数组长度为：");
	scanf("%d", &n);
	printf("\n");
	printf("请输入要查找的数字：");
		scanf("%d", &key);
	int* arr = (int*)malloc(n * sizeof(int));
	int left = 0;
	int right = n - 1;
	for (int i = 0; i < n; i++)
	{
		scanf("%d", &arr[i]);
	}
	int r = int bin_search(arr, left, right, key);
	
		free(arr);
	}
	return r;
}
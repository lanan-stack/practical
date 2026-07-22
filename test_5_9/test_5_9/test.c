#define _CRT_SECURE_NO_WARNINGS 1
#include<stdio.h>
//创建一个整形数组，完成对数组的操作

//实现函数init() 初始化数组为全0
//实现print()  打印数组的每个元素
//实现reverse()  函数完成数组元素的逆置。
//要求：自己设计以上函数的参数，返回值。
void init(int arr[10], int size)
{
	for (int i = 0; i < 10; i++)
	{
		arr[i] = 0;
	}
}
void print(int arr[10], int size)
{
	for (int i = 0; i < size; i++)
	{
		printf("%d ", arr[i]);
	}
}
void reverse(int arr[], int size)
{
	int left = 0;
	int right = size - 1;
	while (left < right)
	{
		int temp = arr[left];
		arr[left] = arr[right];
		arr[right] = temp;
		left++;
		right--;
	}
}
int main()
{
	int n = 0;
	int arr[10] = { 1,2,3,4,5,6,7,8,9,10 };
	n = sizeof(arr)/sizeof(arr[0]);
	printf("初始化前：");
	print(arr, n);
	printf("\n");
	printf("初始化后：");
	init(arr, n);
	print(arr, n);
	int arr2[10] = { 1,2,3,4,5,6,7,8,9,10 };
	printf("\n");
	printf("逆置前：");
	print(arr2, n);
	printf("\n");
	printf("逆置后：");
	reverse(arr2, n);
	for (int i = 0; i < n; i++)
	{
		printf("%d ", arr2[i]);
	}
	return 0;
}
#define _CRT_SECURE_NO_WARNINGS 1
#include<math.h>
#include<stdio.h>
//写一个递归函数DigitSum(n)，输入一个非负整数，返回组成它的数字之和

//例如，调用DigitSum(1729)，则应该返回1 + 7 + 2 + 9，它的和是19
//编写一个函数实现n的k次方，使用递归实现。
//输入：1729，输出：19
int cal1(int n)
{
	if (n == 1 || n == 2)
	{
		return 1;
	}
	return cal1(n - 1) + cal1(n - 2);
}
int cal2(int n)
{
	if (n == 2 || n == 1)
	{
		return 1;
	}
	int a = 1;//f(n-2)
	int b = 1;//f(n-1)
	int c = 0;//f(n)
	for (int i = 3; i <= n; i++)
	{
		c = a + b;
		a = b;
		b = c;
	}
	return c;
}
int main()
{
	int n = 0;
	scanf("%d", &n);
	int r1 = cal1(n);//递归
	int r2 = cal2(n);//非递归
	printf("r1:%d\n", r1);
	printf("r2:%d\n", r2);
	return 0;
}
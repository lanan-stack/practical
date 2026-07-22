#include<math.h>
#include<stdio.h>
//打印菱形图案12行
//求出0～100000之间的所有“水仙花数”并输出。

//“水仙花数”是指一个n位数，其各位数字的n次方之和确好等于该数本身，如:153＝1 ^ 3＋5 ^ 3＋3 ^ 3，则153是一个“水仙花数”
int main()
{
	for (int i = 0; i <= 100000; i++)
	{
		int temp = i;
		int n = 0;
		int sum = 0;
		int digit = 0;
		while (temp != 0)
		{
			n++;
			temp = temp / 10;
		}
		 temp = i;//重新接收原数字
		while (temp != 0)
		{
			digit = temp % 10;
			sum = (int)(sum + pow(digit,n));
			temp = temp / 10;
		}
		if (i == 0)
		{
			sum = 0;
		}
		if (i == sum)
		{
			printf("%d ", i);
		}
	}
	
		
	return 0;
}
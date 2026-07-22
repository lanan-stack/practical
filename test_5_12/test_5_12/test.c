#include<stdio.h>

int main()
{
	int i = 0;
	int j = 0;
	char arr[13][13];
	for (i = 0; i < 13; i++)
	{
		for (int j = 0; j < 13; j++)
		{
			arr[i][j] = ' ';
		}
		
	}
	
	
	for (i = 0; i < 13; i++)
	{
		for ( j = 0; j < 13; j++)
		{
			printf("%c",arr[i][j]);
		}
	}
	return 0;
}
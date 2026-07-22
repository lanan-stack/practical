#define _CRT_SECURE_NO_WARNINGS 1
#include<stdio.h>
#include<stdlib.h>

int main()
{
	int* ptr = (int*)calloc(3, sizeof(int));
	if (ptr == NULL)
	{
		perror("calloc");
		return 1;
	}
	for (int i = 0; i < 3; i++)
	{
		printf("%d ", *(ptr + i));
	}
	free(ptr);
	ptr = NULL;
	return 0;
}
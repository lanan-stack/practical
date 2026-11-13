#include"stack.h"

void stackinit(stack* s)//初始化
{
	assert(s);
	s->arr = (STDataType*)malloc(sizeof(STDataType) * 4);
	if (s->arr == NULL)
	{
		printf("申请失败");
		perror("malloc failed");
		return ;
	}
	s->capacity = 4;
	s->top = 0;
}

void stackdestroy(stack* s)//销毁
{
	assert(s);
	if(s->arr)
	{
		free(s->arr);
		s->arr = NULL;
		s->top = 0;
		s->capacity = 0;
	}
	free(s);
	s = NULL;
}

void stackpush(stack* s, STDataType x)//入栈
{
	assert(s);
	if (s->top > s->capacity - 1)
	{
		STDataType*temp = (STDataType*)realloc(s->arr,sizeof(STDataType) * 2 * s->capacity);
		if (temp == NULL)
		{
			printf("申请失败");
			perror("malloc failed");
			return ;
		}
		//申请成功
		s->arr = temp;
		s->capacity *= 2;
	}
	s->arr[s->top++] = x;
}

void stackpop(stack* s)//出栈，取栈顶元素
{
	assert(s);
	s->top--;
	printf("%d ", s->arr[s->top]);
}

void stacktop(stack* s)//获取栈顶元素并返回
{
	assert(s);
	printf("%d ", s->arr[s->top - 1]);
}

void stacksize(stack* s)// 栈中元素个数
{
	assert(s);
	printf("栈中元素个数为：%d", s->top);
}

void stackempty(stack* s)//检测栈是否为空
{
	assert(s);
	if (s->top == 0)
	{
		printf("栈为空");
	}
	else
		printf("栈不为空");
}
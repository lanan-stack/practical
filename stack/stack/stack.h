#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

typedef int STDataType;
typedef struct stack
{
	STDataType* arr;//指向栈数组空间的指针
	int top;//指向要插入的位置
	int capacity;//容量
} stack;

void stackinit(stack* s);//初始化
void stackdestroy(stack* s);//销毁
void stackpush(stack* s, STDataType x);//入栈
void stackpop(stack* s);//出栈,取栈顶元素
void stacktop(stack* s);//获取栈顶元素并返回
void stacksize(stack* s);//栈中元素个数
void stackempty(stack* s);//检测栈是否为空

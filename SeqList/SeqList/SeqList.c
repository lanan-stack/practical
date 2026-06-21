//函数内容
#include"SeqList.h"

void SQLcheckcapacity(SQL* ps)//扩容//传的结构体
{
	if (ps->size >= ps->capacity)
	{
		SQLDataType* temp = (SQLDataType*)realloc(ps->arr,sizeof(SQLDataType) * ps->capacity * 2);//
		if (temp == NULL)
		{
			perror("malloc failed");
			exit(-1);
		}
		ps->arr = temp;
		ps->capacity *= 2;
	}
}

void SQLInit(SQL*ps)//初始化
{
	assert(ps);
	ps->arr = (SQLDataType*)malloc(sizeof(SQLDataType) * 4);//初始容量为4
		if (ps->arr == NULL)
		{
			perror("malloc failed");
			exit(-1);
		}
		ps->size = 0;//数据个数
		ps->capacity = 4;
}

void SQLDestory(SQL* ps)//销毁（释放）
{
	assert(ps);//断言
	if (ps->arr)
	{
		free(ps->arr);
		ps->size = 0;
		ps->capacity = 0;
		ps->arr = NULL;
	}
}
void SQLPushFront(SQL* ps, SQLDataType x)//头插
{
	assert(ps);
	 SQLcheckcapacity(ps);
	
	 for (int i = ps->size; i > 0; i--)
	 {
		 ps->arr[i] = ps->arr[i - 1];

	}
	 ps->arr[0] = x;
	 ps->size++;
}

void SQLPrint(SQL* ps)//打印
{
	for (int i = 0; i < ps->size; i++)
	{
		printf("%d ", ps->arr[i]);
	}
	printf("\n");
}

void SQLPopFront(SQL* ps)//头删
{
	assert(ps);
	for (int i = 0; i < ps->size-1; i++)
	{
		ps->arr[i] = ps->arr[i + 1];
	}
	ps->size--;
}

void SQLPushBack(SQL* ps, SQLDataType x)//尾插
{
	assert(ps);
	SQLcheckcapacity(ps);

	ps->arr[ps->size] = x;
	ps->size++;
}

void SQLPopBack(SQL* ps)//尾删
{
	assert(ps);
	assert(ps->size > 0);
	ps->size--;
}

void SQLInsert(SQL* ps, int i, SQLDataType x)//在顺序表指定位置插入x
{
	assert(ps);
	assert(i > 0 && i <= ps->size);
	SQLcheckcapacity(ps);
	for (int j = ps->size; j > i - 1; j--)
	{
		ps->arr[j] = ps->arr[j-1];
	}
	ps->arr[i - 1] = x;
	ps->size++;
}

void SQLErase(SQL* ps, int i)//在指定位置删除
{
	assert(ps);
	assert(i > 0 && i <= ps->size);
	for (int j = i; j < ps->size; j++)
	{
		ps->arr[j - 1] = ps->arr[j];
	}
	ps->size--;
}
int SQLFind(SQL* ps, SQLDataType x)//查找
{
	int i = 0;
	for ( i = 0; i < ps->size;i++)
	{
		if (ps->arr[i] == x)
		{
			break;
		}
	}
	if (i<ps->size)
	{
		printf("找到了，在顺序表的第%d个位置", i + 1);
	}
	else
	{
		printf("顺序表中没有这个元素");
	}
}
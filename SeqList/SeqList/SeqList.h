#define INIT_CAPACITY 4;//函数声明
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
typedef int SQLDataType;
typedef struct SqList
{
	SQLDataType* arr;
	int capacity;//空间容量
	int size;//数据个数
}SQL;
SQL sql;
void SQLInit(SQL* ps);//初始化
void SQLDestory(SQL* ps);//释放
void SQLPrint(SQL* ps);//打印
void SQLPushFront(SQL* ps, SQLDataType x);//头插
void SQLcheckcapacity(SQL* ps);//扩容
void SQLPrint(SQL* ps);//打印
void SQLPopFront(SQL* ps);//头删
void SQLPushBack(SQL* ps, SQLDataType x);//尾插
void SQLPopBack(SQL*ps);//尾删
void SQLInsert(SQL* ps, int i, SQLDataType x);//在顺序表指定位置插入x
void SQLErase(SQL* ps, int i);//在顺序表指定位置删除
int SQLFind(SQL* ps, SQLDataType x);//查找
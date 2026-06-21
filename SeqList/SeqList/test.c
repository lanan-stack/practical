#include"SeqList.h"//≤‚ ‘
void test()
{
	
	SQLInit(&sql);
	SQLPushBack(&sql,1);
	SQLPushBack(&sql, 2);
	SQLPushBack(&sql, 3);
	SQLPushBack(&sql, 4);
	SQLPrint(&sql);
	SQLFind(&sql, 4);
	
	SQLDestory(&sql);
}
int main()
{
	
	test();
	return 0;
}


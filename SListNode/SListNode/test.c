#include"SListNode.h"

void test()
{
	SLN* plist = createplist();//创建头结点
	
	SLNPushBack(plist, 1);
	SLNPushBack(plist, 2);
	SLNPushBack(plist, 3);
	SLNPushBack(plist, 4);
	SLNPrint(plist);
	SLNFind(plist, 1);
}

int main()
{
	test();
}
#include"DCList.h"

void test()
{
	DCListNode* DCL;
	DCListInit1(&DCL);
	DCLPushBack(DCL, 1);
	DCLPushBack(DCL, 2);
	DCLPushBack(DCL, 3);
	DCLPushBack(DCL, 4);

	/*DCLPopBack(DCL);
	DCLPrint(DCL);
	DCLPopBack(DCL);
	DCLPrint(DCL);
	DCLPopBack(DCL);
	DCLPrint(DCL);
	DCLPopBack(DCL);*/
	DCLPrint(DCL);
	DCListDestory(&DCL);
}
int main()
{

	test();
}
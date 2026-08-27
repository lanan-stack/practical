#include"stack.h"


void test()
{
	stack* s = (stack*)malloc(sizeof(stack));
	stackinit(s);
	stackpush(s, 1);
	stackpush(s, 2);
	stackpush(s, 3);
	stackpush(s, 4);
	//stackpop(s);
	//stackpop(s);
	//stackpop(s);
	//stackpop(s);
	//stacktop(s);
	//stacksize(s);
	stackempty(s);

	stackdestroy(s);
}
int main()
{
	
	test();
	return 0;
}
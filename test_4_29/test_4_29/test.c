#include<stdio.h>


 int Add(int x, int y)
 {
	      return x + y;
 }

 int main()
 {
	 int(*pf3)(int, int) = Add;

	 printf("%d\n", (*pf3)(2, 3));
	 printf("%d\n", pf3(3, 5));

	 return 0;
 }
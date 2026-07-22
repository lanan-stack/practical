#include <stdio.h>
 //代码1 - 测试无const修饰的情况
  void test1()
  {
	      int n = 10;
	      int m = 20;
	      int* p = &n;
	    * p = 20;//ok?
	      p = &m; //ok?
	  }

  //代码2 - 测试const放在*的左边情况
  void test2()
  {
	      int n = 10;
	      int m = 20;
	      const int* p = &n;
	    * p = 20;//ok?
	      p = &m; //ok?
  }

  //代码3 - 测试const放在*的右边情况
  void test3()
  {
	      int n = 10;
      int m = 20;
	      int* const p = &n;
	 int * p = 20; //ok?
	      p = &m;  //ok?
  }

  //代码4 - 测试*的左右两边都有const
  void test4()
  {
	      int n = 10;
	      int m = 20;
	      int const* const p = &n;
	      * p = 20; //ok?
	      p = &m;  //ok?
	  }

  int main()
  {
	      //测试无const修饰的情况
		      test1();
	     //测试const放在*的左边情况
		      test2();
	      //测试const放在*的右边情况
		      test3();
	      //测试*的左右两边都有const
		      test4();
	      return 0;
	  }
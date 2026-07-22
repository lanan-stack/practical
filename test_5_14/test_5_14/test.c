#include<stdio.h>

int main()
{
	struct S3
		  {
		      double d;
		      char c;
		      int i;
		  };
	  printf("%zu\n", sizeof(struct S3));

	  struct S4
		    {
		        char c1;
		        struct S3 s3;
		        double d;
		    };
	    printf("%zu\n", sizeof(struct S4));


	return 0;
}
#include<iostream>
using namespace std;
// 计算?下A/B/C实例化的对象是多?？
class A
{
public:
	void Print()
	{
		cout << _ch << endl;
	}
private:
	char _ch;
	int _i;
};
class B
{
public:
	void Print()
	{
		//...
	}
};
class C
{
};
int main()
{
	A a;
	B b;
	C c;
	cout << sizeof(a) << endl;
	cout << sizeof(b) << endl;
	cout << sizeof(c) << endl;
	return 0;
}
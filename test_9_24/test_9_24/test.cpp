#include<iostream>
using std::cout;
using std::endl;
class Date
{
public:
	void print()
	{
		cout << _year << "/" << _month << "/" << _day << endl;
	}
private:
	int _year;
	int _month;
	int _day;
};

int main()
{
	Date d1;
	d1.print();
	return 0;
}
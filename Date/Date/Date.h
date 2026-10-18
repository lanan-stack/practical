#pragma once
#include<iostream>
#include<assert.h>
#include<stdbool.h>
using std::cout;
using std::endl;

class Date
{
public:
	Date& operator+=(int day);//d1 += 5,返回对象本身，用引用
	Date operator+(int day) const;//+不改变对象，用引用会造成野引用（返回局部对象的引用）
	Date& operator-=(int day);
	Date operator-(int day) const;//-不改变对象
	Date& operator++();//前置++,返回本体
	Date& operator--();//前置--，返回本体
	Date operator++(int);//后置++，返回之前的自己(临时对象）
	Date operator--(int);//后置--，，返回之前的自己(临时对象）
	friend std::ostream& operator<<(std::ostream& out, const Date& d);
	friend std::istream& operator>>(std::istream& in,  Date& d);
	explicit Date(int year = 2026, int month = 10, int day = 4)//默认构造函数，全缺省。explicit防隐式转换
		:_year(year)
	    ,_month(month)
	    ,_day(day)
	{
		
	}

	 Date(const Date& d)//d3(d1)
	 :_year(d._year)
	 ,_month(d._month)
	 ,_day(d._day)
	{
		
	}

	 void Print() const//打印
	 {
		 if (!CheckDate(_year, _month, _day))
		 {
			 cout << "日期非法" << endl;
			 return;
		 }
		 else
		 {
			 cout << _year << "/" << _month << "/" << _day << endl;
		 }
	 }

	 bool CheckDate(int year,int month, int day) const//检查是否合法
	 {
		 if (month < 1 || month>12 || day < 1 || year<0)
		 {
			 return false;//不合法
		 }
		 else
		 {
			 return day <= GetMonthDay(year, month);;//合法
		 }
	 }

	static int GetMonthDay(int year, int month)//获取天数
	 {
		 assert(month > 0 && month < 13);
		 static int monthday[13] = { -1,31,28,31,30,31,30,31,31,30,31,30,31 };
		 if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
		 {
			 return 29;
		 }
		 return monthday[month];
	 }
private:
	int _year;
	int _month;
	int _day;
};
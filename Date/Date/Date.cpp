#include"Date.h"

Date Date::operator+(int day) const
{
	Date temp(*this);//this为指针
	temp += day;
	return temp;
}
Date& Date::operator+=(int day)
{
	if (day < 0)
	{
		return *this -= -day;
	}
	int maxday = GetMonthDay(_year,_month);
	if (_day + day <= maxday)
	{
		_day = _day + day;
		return *this;
	}
	else
	{ 
		_day = _day + day;
		while(_day>maxday)
	 {
		 _day -= maxday;
		++_month;
		if (_month > 12)
		{
			++_year;
			_month = 1;
		}
		maxday = GetMonthDay(_year, _month);
	 }
	  return *this;

	}
}

Date Date::operator-(int day) const
{
	Date temp(*this);
	 temp -= day;
	 return temp;
}
Date& Date::operator-=(int day)
{
	assert(day != INT_MIN);
	if (day < 0)
	{
		return *this += -day;//如果 day == INT_MIN，那么 -day 会溢出，因为 INT_MIN 的绝对值比 INT_MAX 大 1
	}
	_day = _day - day;
	if(_day>0)
	{
		return *this;
	}
	else
	{ while(_day<=0)
	 {
		--_month;
		if (_month < 1)
		{
			--_year;
			_month = 12;
		}
		int maxday = GetMonthDay(_year, _month);
		_day += maxday;
	 }
	return *this;
	}

	
}

Date& Date::operator++()//前置++
{
	*this += 1;
	return *this;
}

Date Date::operator++(int)//后置++
{
	Date temp(*this);
	*this += 1;
	return temp;
}

Date& Date::operator--()//前置--
{
	*this -= 1;
	return *this;
}

Date Date::operator--(int)//后置--
{
	Date temp(*this);
	*this -= 1;
	return temp;
}

std::ostream& operator<<(std::ostream& out, const Date& d)//可用Print，也可用这个，不过友元慎用
{
	out << d._year << "/" << d._month << "/" << d._day;
	return out;
}

std::istream& operator>>(std::istream& in, Date& d)
{
	in >> d._year >> d._month >> d._day;
	return in;
}
#pragma once
#include<iostream>
#include<assert.h>
#include<string.h>
#include<stdbool.h>

using std::cout;
using std::endl;

namespace lem
{
	class MyString
	{
	public:
		//迭代器
		typedef char* iterator;
		typedef const char* const_iterator;

		iterator begin();
		iterator end();

		const_iterator begin() const;
		const_iterator end() const;

		
		

		//构造
		
		//MyString();

		MyString(const char* str = "");//默认值
		const char* c_str() const;
		MyString(const MyString& s);//s2(s1)
		
		~MyString();

		void swap(MyString& s);
		size_t size() const;
		char& operator[](size_t i);//可读可修改
		const char& operator[](size_t i) const;//可读不可修改
		void operator=( MyString& s);//赋值运算符

		void reserve(size_t n);
		void resize(size_t n,char val = '\0');
		void pop_back();
		void push_back(char ch);
		void append(const char* str);
		bool empty();

	private:
		char* _str;
		size_t _size;
		size_t _capacity;
	};
}
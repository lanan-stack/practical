#include"MyString.h"


namespace lem
{
	bool MyString::empty()
	{
		if (_size == 0)
		{
			return true;
		}
		return false;
	}



	MyString::iterator MyString::begin()
	{
		return _str;
	}

	MyString::iterator MyString::end()
	{
		return _str + _size;
	}

	MyString::const_iterator MyString::begin() const//后const修饰this
	{
		return _str;
	}

	MyString::const_iterator MyString::end() const
	{
		return _str + _size;
	}

	/*MyString::MyString()
		:_str(nullptr)
		, _size(0)
		, _capacity(0)
	{

	}*/

	//s2(s1)
	MyString::MyString(const char* str)
	{
		assert(str);
		_size = strlen(str);
		_str = new char[_size + 1];
		_capacity = _size;
		memcpy(_str, str,_size);
		_str[_size] = '\0';
	}

	const char* MyString::c_str() const
	{
		return _str;
	}

	MyString::MyString(const MyString& s)//s2(s1)
		:_str(nullptr)
	{
		_size = strlen(s.c_str());
		_str = new char[_size + 1];
		memcpy(_str, s._str,_size);//拷贝数据
		_str[_size] = '\0';
		_capacity = _size;
	}

	MyString::~MyString()
	{
		delete[]_str;
		_str = nullptr;
		_size = _capacity = 0;
	}

	size_t MyString::size() const
	{
		return _size;
	}

	char& MyString::operator[](size_t i)
	{
		//return *(_str + i);
		return _str[i];
	}

	const char& MyString::operator[](size_t i) const//可读不可修改
	{
		//return *(_str + i);
		return _str[i];
	}

	void MyString::swap(MyString& s)
	{
		std::swap(_str, s._str);
		std::swap(_size, s._size);
		std::swap(_capacity, s._capacity);
	}

	void MyString::operator=( MyString& s)
	{
		MyString::swap(s);

	}

	void MyString::reserve(size_t n)
	{
		//扩容
		if (n > _capacity)
		{
			char* temp = new char[n + 1];
			if (_str != nullptr)
			{
				memcpy(temp, _str, _size);
			}
			temp[_size] = '\0';
			std::swap(_str, temp);
			delete[] temp;
			_capacity = n;
		}
	}

	void MyString::resize(size_t n,char val)
	{
		if (n > _capacity)
		{
			reserve(n);
			_capacity = n;
		}
		if (n > _size)
		{
			for (int i = _size; i < n; i++)
			{
				_str[i] = val;
			}
			_str[_size] = val;
			_size = n;
		}
		//n<=_size
		else
		{
			_str[n] = '\0';
			_size = n;
		}
	}

	void MyString::pop_back()
	{
		assert(_size);
		_str[_size - 1] = '\0';
		_size--;
	}

	void MyString::push_back(char ch)
	{
		if (_size == _capacity)
		{
			size_t newcapacity = _capacity == 0 ? 16 : 2 * _capacity;
			reserve(newcapacity);
		}
		_str[_size] = ch;
		_size++;
		_str[_size] = '\0';
	}

	void MyString::append(const char* str)
	{
		assert(str);

		size_t len = strlen(str);      // 长度必须在扩容前取：扩容后 str 可能已经悬空
		if (len == 0)
		{
			return;                    // 追加空串，什么也不用做
		}

		// 【自引用防护】
		// str 有可能指向本对象自己的缓冲区，例如：
		//     s.append(s.c_str());
		//     s.append(s.c_str() + 2);
		// 如果先扩容，旧缓冲区会被 delete[] 释放，str 就变成悬空指针，
		// 后面再 memcpy 读的就是已经释放的内存（未定义行为）。
		//
		// 做法：先判断 str 是否落在本对象的缓冲区 [_str, _str + _size] 里；
		// 如果是自引用，就在扩容之前把这段内容备份到一块新的临时内存里，
		// 等扩容完成后再从备份里拷贝，最后 delete[] 掉备份。
		char* backup = nullptr;          // 不是自引用时保持空指针
		if (str >= _str && str <= _str + _size)
		{
			backup = new char[len + 1];
			memcpy(backup, str, len + 1);   // 连结尾的 '\0' 一起备份
			str = backup;                   // 后面统一从备份里拷贝
		}

		size_t old_size = _size;
		reserve(old_size + len);         // 只申请空间；append 不需要 resize 的“填充字符”语义
		memcpy(_str + old_size, str, len);
		_size = old_size + len;          // 更新长度
		_str[_size] = '\0';              // 补终止符

		delete[] backup;                 // backup 是 nullptr 时，delete[] 也是合法的空操作
	}



}

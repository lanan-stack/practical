#pragma once
#include<iostream>
#include<assert.h>
#include<vector>
#include<initializer_list>
#include<stdbool.h>

using std::cout;
using std::endl;
namespace lem
{
	template<class T> class MyVector;

	template<class T>
	class MyVector
	{
	public:
		//构造
		typedef T* iterator;
		typedef const T* const_iterator;

		bool empty() const
		{
			if (_start == _finish)
			{
				return true;
			}
			return false;
		}

		size_t capacity() const
		{
			if (_start == _endofstorage)//空
				return 0;
			return _endofstorage - _start;
		}

		size_t size() const
		{
			if (_start == _finish)//空
				return 0;
			return _finish - _start;
		}

		void reserve(size_t n)//只扩不缩
		{
			if (n > capacity())
			{
				size_t old_size = size();
				T* temp = new T[n];
				//有数据

				for (size_t i = 0; i < old_size; i++)
				{
					temp[i] = _start[i];
				}
				delete[] _start;
				_start = temp;
				_finish = _start + old_size;
				_endofstorage = _start + n;

			}
		}

		//用iterator支持链式调用，多次插入
		iterator insert(iterator pos, const T& val)
		{
			assert(pos >= _start && pos <= _finish);

			if (_finish == _endofstorage)
			{
				size_t old_size = pos - _start;
				reserve(capacity() == 0 ? 4 : 2 * capacity());
				pos = _start + old_size;
			}

			
			iterator end = _finish;
			while (end != pos)
			{
				*end = *(end - 1);
				--end;
			}
			*pos = val;
			_finish++;
			return pos;
		}

		iterator erase(iterator pos)
		{
			assert(pos >= _start && pos < _finish);
			iterator temp = pos;
			while (temp != _finish - 1)
			{
				*temp = *(temp + 1);
				++temp;
			}
			_finish--;
			return pos;
		}

		iterator begin()
		{
			return _start;
		}

		iterator end()
		{
			return _finish;
		}

		const_iterator begin() const
		{
			return _start;
		}

		const_iterator end() const
		{
			return _finish;
		}

		/*MyVector()
			:_start(nullptr)
			,_finish(nullptr)
			,_endofstorage(nullptr)
		{

		}*/

		//推荐用这个
		//性能高，强制编译器生成最优的默认构造，还可防止初始化列表漏初始化成员变量
		MyVector() = default;

		//vector<T> v{1,2,3} vector<T> v = {1,2,3}
		MyVector(std::initializer_list <T> vt)
		{
			reserve(vt.size());//先扩容
			for (const auto& v : vt)//用引用&，避免拷贝的开销，提升效率
			{
				push_back(v);
			}
		}

		//v2(v1)
		MyVector(const MyVector<T>& vt)
		{
			reserve(vt.capacity());//扩容
			for (const auto& v : vt)
			{
				push_back(v);
			}
		}

		//v2 = v1
		MyVector<T>& operator=(const MyVector<T>& vt)
		{
			if (this != &vt)
			{
				T* temp = new T[vt.capacity()];
				for (size_t i = 0; i < vt.size(); i++)
				{
					temp[i] = vt._start[i];
				}
				delete[] _start;
				//复制成功
				_start = temp;
				_finish = _start + vt.size();
				_endofstorage = _start + vt.capacity();
			}
			return *this;
		}

		~MyVector()
		{
			if (_start)
			{
				delete[] _start;
				_start = _finish = _endofstorage = nullptr;
			}
		}

		
		

		
		void resize(size_t n, T val = T())
		{
			if (n > size())
			{
				if (n > capacity())
				{
					//扩容
					reserve(n);
					while (_finish != _endofstorage)
					{

						*_finish = val;
						++_finish;
					}
				}
				else
				{
					//size<n<=capacity
					while (_finish != (_start + n))
					{
						*_finish = val;
						++_finish;
					}
				}
			}
			else
			{
				//n<=size
				_finish = _start + n;
			}
		}


		
		void push_back(const T& x)
		{
			if (_finish == _endofstorage)
			{
				//满了
				reserve(capacity() == 0 ? 4 : 2 * capacity());
			}
			*_finish = x;
			++_finish;
		}

		
		void pop_back()
		{
			assert(!empty());
			--_finish;
		}


		

		/*template<class T>
		void MyVector<T>::Print(const MyVector<T>& v)// &防止传值引用，传值会拷贝整个实参，避免拷贝开销
		{
			for (auto& vt : v)
			{
				cout << vt;
			}
			cout << endl;
		}*/

		
		void Print() const
		{
			for (const auto& vt : *this)
			{
				cout << vt<<" ";
			}
			cout << endl;
		}

	private:
		iterator _start = nullptr;
		iterator _finish = nullptr;
		iterator _endofstorage = nullptr;

	};
}
// ============================================================================
// main.cpp —— lem::MyString 的测试代码
// ----------------------------------------------------------------------------
// 使用说明：
//   1. 每个 test_xxx() 针对一个接口（或一组相关行为），函数上方的注释说明了
//      这个用例在验证什么、期望的语义是什么。
//   2. 断言用本文件里的 CHECK / CHECK_STREQ 宏：失败只打印 [FAIL] 和行号，
//      不会中断程序；全部跑完后统计“断言总数 / 通过 / 失败”，有失败时
//      main 返回 1（以后想接 CI 也方便）。
//   3. 文末的 RUN_BUG_CASES 控制“当前实现会直接崩溃”的用例：
//      默认 0 = 先不跑；等你按审查意见改好 MyString 后改成 1 再跑一次。
//
// 按当前 MyString.cpp 的实现，预期会失败的断言：
//   * test_resize 第 2 段 —— resize(n, val) 少写结尾 '\0'
//     （循环写成了 i <= n，把第 n 个字符覆盖掉了）
//   * 打开 RUN_BUG_CASES 后：
//       - test_pop_back 第 4 段 —— 空串 pop_back，_size - 1 下溢
//       - test_copy_assign    —— 没有 operator=，默认浅拷贝 -> double free
//   * test_reserve 能通过，但 reserve 里 "delete temp" 应为 "delete[] temp"，
//     对 char 这种平凡类型只是“碰巧没崩”，仍属于未定义行为。
// ============================================================================
#include "MyString.h"

#include <iostream>  // std::cout / std::endl

#include <cstring>      // strcmp
#include <cctype>       // toupper / tolower
#include <cstddef>      // std::ptrdiff_t
#include <type_traits>  // std::is_same_v

// 0 = 不跑“现在会崩溃”的用例；MyString 修好之后改成 1（也可以编译时加 -DRUN_BUG_CASES=1）
#ifndef RUN_BUG_CASES
#define RUN_BUG_CASES 0
#endif

// ---------------------------------------------------------------------------
// 极简测试框架
// ---------------------------------------------------------------------------
namespace
{
	int g_total = 0;   // 断言总数
	int g_pass = 0;    // 通过数
	int g_fail = 0;    // 失败数

	// 统一断言出口：成功时静默，失败时打印行号 + 表达式原文
	void check_impl(bool ok, const char* what, int line)
	{
		++g_total;
		if (ok)
		{
			++g_pass;
		}
		else
		{
			++g_fail;
			std::cout << "  [FAIL] line " << line << ": " << what << std::endl;
		}
	}

	// 两个 C 字符串内容是否相同（用 strcmp，因此要求末尾 '\0' 是正确的）
	bool same_text(const char* a, const char* b)
	{
		return a != nullptr && b != nullptr && strcmp(a, b) == 0;
	}
}

// 断言表达式为真
#define CHECK(expr) check_impl((expr), #expr, __LINE__)

// 断言 c_str() 的内容等于期望字符串；失败时额外打印实际值/期望值
#define CHECK_STREQ(actual, expect)                                        \
	do {                                                                   \
		const char* _a = (actual);                                         \
		const char* _e = (expect);                                         \
		if (same_text(_a, _e))                                             \
		{                                                                  \
			check_impl(true, "", __LINE__);                                \
		}                                                                  \
		else                                                               \
		{                                                                  \
			check_impl(false, "string content mismatch", __LINE__);        \
			std::cout << "         actual: [" << _a << "]  expect: [" << _e << "]" << std::endl; \
		}                                                                  \
	} while (0)

// 断言对象的不变式：第 size() 个字符必须是 '\0'（c_str 的终止符）。
// 这里刻意用 operator[] 而不是 c_str()/strlen：
//   - 下标 size() 一定还在 capacity+1 的分配范围内，即使实现漏写 '\0'
//     也不会在本断言里造成越界读，只会让这条断言失败；
//   - 如果直接用 c_str() 打印/strlen，实现漏写 '\0' 时会越界读到堆上别的
//     数据（ASan 会直接报 heap-buffer-overflow），程序就崩了。
#define CHECK_TERMINATED(obj)                                              \
	do {                                                                   \
		const size_t _n = (obj).size();                                    \
		check_impl((obj)[_n] == '\0',                                      \
			"char at index size() must be '\\0' (terminator)", __LINE__);  \
	} while (0)

// ---------------------------------------------------------------------------
// 1) 构造函数：默认构造 / const char* / 隐式转换 / 长串
// ---------------------------------------------------------------------------
void test_construct()
{
	// MyString.h 里只保留了带默认参数的构造 MyString(const char* str = "")，
	// 所以 `MyString s;` 会走默认参数。若同时再写一个无参构造，`MyString s;`
	// 会因为两个候选一样好而产生二义性编译错误（原 main.cpp 的注释就是这个意思）。
	lem::MyString s0;
	CHECK(s0.size() == 0);
	CHECK_STREQ(s0.c_str(), "");
	CHECK_TERMINATED(s0);

	// 普通字符串（带空格）
	lem::MyString s1("hello world");
	CHECK(s1.size() == 11);
	CHECK_STREQ(s1.c_str(), "hello world");
	CHECK_TERMINATED(s1);

	// 构造函数没有 explicit，所以允许隐式转换：const char* -> MyString
	lem::MyString s2 = "abc";
	CHECK(s2.size() == 3);
	CHECK_STREQ(s2.c_str(), "abc");

	// 空 C 串
	lem::MyString s3("");
	CHECK(s3.size() == 0);
	CHECK_STREQ(s3.c_str(), "");
	CHECK_TERMINATED(s3);

	// 长串：300 个字符，顺便验证 strlen/new/memcpy 没有长度问题
	char longbuf[301];
	for (int i = 0; i < 300; ++i)
	{
		longbuf[i] = char('a' + i % 26);
	}
	longbuf[300] = '\0';

	lem::MyString s4(longbuf);
	CHECK(s4.size() == 300);
	CHECK_STREQ(s4.c_str(), longbuf);
	CHECK_TERMINATED(s4);
}

// ---------------------------------------------------------------------------
// 2) 拷贝构造：必须是深拷贝（两块独立内存，内容相同，修改互不影响）
// ---------------------------------------------------------------------------
void test_copy_construct()
{
	lem::MyString a("hello");
	lem::MyString b(a);                 // s2(s1) 的写法

	CHECK(b.size() == a.size());
	CHECK_STREQ(b.c_str(), "hello");
	CHECK(a.c_str() != b.c_str());      // 指针不同 => 各自 new 了一块内存

	// 深拷贝验证：改 b[0] 不能影响 a
	b[0] = 'H';
	CHECK_STREQ(a.c_str(), "hello");
	CHECK_STREQ(b.c_str(), "Hello");

	// 空串拷贝
	lem::MyString e;
	lem::MyString e2(e);
	CHECK(e2.size() == 0);
	CHECK_STREQ(e2.c_str(), "");
	CHECK_TERMINATED(e2);
}

// ---------------------------------------------------------------------------
// 3) 迭代器：begin/end 的 const 与非 const 重载、范围 for、指针差
// ---------------------------------------------------------------------------
void test_iterators()
{
	lem::MyString s("hello");

	// 非 const 版 begin()/end() 返回 char*，可以改字符：这里全部转大写
	for (lem::MyString::iterator it = s.begin(); it != s.end(); ++it)
	{
		*it = char(toupper(static_cast<unsigned char>(*it)));
	}
	CHECK_STREQ(s.c_str(), "HELLO");

	// end - begin 应该正好等于 size()（end 指向 '\0' 的位置）
	CHECK(s.end() - s.begin() == static_cast<std::ptrdiff_t>(s.size()));

	// 范围 for 底层就是 begin()/end()，顺便验证一下
	for (char& c : s)
	{
		c = char(tolower(static_cast<unsigned char>(c)));
	}
	CHECK_STREQ(s.c_str(), "hello");

	// const 对象走 const 重载，begin()/end() 必须返回 const_iterator（const char*）
	const lem::MyString& cs = s;
	static_assert(std::is_same_v<decltype(cs.begin()), lem::MyString::const_iterator>,
		"const object should call the const overload of begin()");

	size_t count = 0;
	for (lem::MyString::const_iterator it = cs.begin(); it != cs.end(); ++it)
	{
		++count;
	}
	CHECK(count == cs.size());

	// 迭代器确实指向同一块缓冲区：*begin 就是第 0 个字符
	CHECK(*s.begin() == 'h');

	// 空串的迭代器：begin == end
	lem::MyString empty;
	CHECK(empty.begin() == empty.end());
}

// ---------------------------------------------------------------------------
// 4) operator[]：非 const 可读可写，const 只读；用 '\0' 作为遍历终点
// ---------------------------------------------------------------------------
void test_subscript()
{
	lem::MyString s("abc");

	// 非 const 版本可读可写
	CHECK(s[0] == 'a');
	s[0] = 'A';
	CHECK(s[0] == 'A');
	CHECK_STREQ(s.c_str(), "Abc");

	// const 版本只能读（这里写 const 对象取下标）
	const lem::MyString& cs = s;
	CHECK(cs[1] == 'b');
	// cs[1] = 'B';   // 取消注释会编译报错，正好证明 const 版返回 const char&

	// 原 main.cpp 里的写法：for (i = 0; s[i] != '\0'; i++)
	// 因为尺寸由 _size 维护，所以正常字符串上这个循环次数应等于 size()
	size_t len = 0;
	for (size_t i = 0; s[i] != '\0'; ++i)
	{
		++len;
	}
	CHECK(len == s.size());
	CHECK(s.size() == 3);
}

// ---------------------------------------------------------------------------
// 5) reserve：只扩不缩、内容保留、预留之后继续追加
// ---------------------------------------------------------------------------
void test_reserve()
{
	lem::MyString s("hello");

	// 扩容：内容不变、size 不变
	s.reserve(100);
	CHECK_STREQ(s.c_str(), "hello");
	CHECK(s.size() == 5);
	CHECK_TERMINATED(s);

	// reserve 不应缩容：再 reserve 一个比当前容量小的值，不应重新分配（指针不变）
	const char* p = s.c_str();
	s.reserve(10);
	CHECK(s.c_str() == p);

	// 预留过容量之后再追加，不应再触发扩容
	s.append(" world");
	CHECK_STREQ(s.c_str(), "hello world");
	CHECK(s.size() == 11);
	CHECK_TERMINATED(s);

	// 空串 reserve(0)：什么都不该发生，也不该崩
	lem::MyString e;
	e.reserve(0);
	CHECK_STREQ(e.c_str(), "");
}

// ---------------------------------------------------------------------------
// 6) resize：扩大（默认 '\0' 填充 / 指定字符填充）、缩小、缩到 0
// ---------------------------------------------------------------------------
void test_resize()
{
	// 6.1 扩大，默认用 '\0' 填充：size 变大，但 c_str() 仍在原来的内容处结束
	{
		lem::MyString s("abc");
		s.resize(6);
		CHECK(s.size() == 6);
		CHECK_STREQ(s.c_str(), "abc");
		CHECK_TERMINATED(s);
		for (size_t i = 3; i < 6; ++i)
		{
			CHECK(s[i] == '\0');
		}
	}

	// 6.2 扩大，用指定字符填充：std::string 语义是
	//     "abc" + resize(6,'x')  => size=6，内容 abcxxx，末尾仍有 '\0'
	{
		lem::MyString s("abc");
		s.resize(6, 'x');
		CHECK(s.size() == 6);
		for (size_t i = 3; i < 6; ++i)
		{
			CHECK(s[i] == 'x');
		}
		// 关键断言：第 size() 个字符必须还是 '\0'。
		// 当前实现把填充循环写成 for (int i = _size; i <= n; i++)，
		// 会连第 n 个字符一起写成 val，且之后没有再补 '\0'，
		// 所以这条会失败；此时若调用 c_str()/strlen 就是越界读。
		CHECK_TERMINATED(s);
	}

	// 6.3 缩小：size 变小、原内容被截断
	{
		lem::MyString s("abcdef");
		s.resize(3);
		CHECK(s.size() == 3);
		CHECK_STREQ(s.c_str(), "abc");
		CHECK_TERMINATED(s);
	}

	// 6.4 缩到 0
	{
		lem::MyString s("abc");
		s.resize(0);
		CHECK(s.size() == 0);
		CHECK_STREQ(s.c_str(), "");
		CHECK_TERMINATED(s);
	}

	// 6.5 resize 之后状态仍然合法，能继续 push_back
	{
		lem::MyString s("ab");
		s.resize(5, 'c');
		s.push_back('d');
		CHECK(s.size() == 6);
		CHECK(s[2] == 'c');
		CHECK(s[3] == 'c');
		CHECK(s[4] == 'c');
		CHECK(s[5] == 'd');
		CHECK_TERMINATED(s);
	}
}

// ---------------------------------------------------------------------------
// 7) push_back：空串开始追加、连续追加触发多次扩容、末尾字符与 '\0'
// ---------------------------------------------------------------------------
void test_push_back()
{
	// 7.1 空串追加一个字符
	{
		lem::MyString s;
		s.push_back('a');
		CHECK(s.size() == 1);
		CHECK_STREQ(s.c_str(), "a");
		CHECK_TERMINATED(s);
	}

	// 7.2 连续追加 100 次：会经历 16 -> 32 -> 64 -> 128 多次扩容，
	//     每一步都要检查“长度、末尾字符、'\0' 终止符、旧内容没丢”
	{
		lem::MyString s;
		for (int i = 0; i < 100; ++i)
		{
			const char ch = char('a' + i % 26);
			s.push_back(ch);
			CHECK(s.size() == static_cast<size_t>(i + 1));
			CHECK(s[i] == ch);
			CHECK_TERMINATED(s);
		}
		CHECK(s[0] == 'a');         // 最早写入的字符还在
		CHECK(s[25] == 'z');        // 第 26 个字符
	}

	// 7.3 已有内容之后追加
	{
		lem::MyString s("hello");
		s.push_back('!');
		CHECK(s.size() == 6);
		CHECK_STREQ(s.c_str(), "hello!");
	}
}

// ---------------------------------------------------------------------------
// 8) pop_back：正常删除、一直删到空、删完再追加
// ---------------------------------------------------------------------------
void test_pop_back()
{
	// 8.1 正常删除最后一个字符
	{
		lem::MyString s("abc");
		s.pop_back();
		CHECK(s.size() == 2);
		CHECK_STREQ(s.c_str(), "ab");
		CHECK_TERMINATED(s);
	}

	// 8.2 一直删到空串（注意：不要对空串再 pop_back，见 8.4）
	{
		lem::MyString s("abc");
		while (s.size() > 0)
		{
			s.pop_back();
		}
		CHECK(s.size() == 0);
		CHECK_STREQ(s.c_str(), "");
		CHECK_TERMINATED(s);
	}

	// 8.3 删除之后再追加，内容要正确衔接
	{
		lem::MyString s("ab");
		s.pop_back();
		s.push_back('c');
		CHECK_STREQ(s.c_str(), "ac");
	}

#if RUN_BUG_CASES
	// 8.4 已知问题：pop_back 没有判空，空串时 _size - 1 下溢成 SIZE_MAX，
	//     会往 _str[SIZE_MAX] 写 '\0'，直接堆破坏/崩溃
	//     （ASan 报 heap-buffer-overflow WRITE of size 1）。
	//     等 MyString::pop_back 加上 "if (_size == 0) return;" 之后再打开。
	{
		lem::MyString s;
		s.pop_back();
		CHECK(s.size() == 0);
	}
#endif
}

// ---------------------------------------------------------------------------
// 9) append：基本追加、空串、连续追加跨多次扩容、与 push_back 混用
// ---------------------------------------------------------------------------
void test_append()
{
	// 9.1 基本追加
	{
		lem::MyString s("hello");
		s.append(" world");
		CHECK(s.size() == 11);
		CHECK_STREQ(s.c_str(), "hello world");
		CHECK_TERMINATED(s);
	}

	// 9.2 追加空串：什么都不该变
	{
		lem::MyString s("abc");
		s.append("");
		CHECK(s.size() == 3);
		CHECK_STREQ(s.c_str(), "abc");
	}

	// 9.3 连续追加，跨越多次扩容
	{
		lem::MyString s;
		for (int i = 0; i < 20; ++i)
		{
			s.append("ab");
		}
		CHECK(s.size() == 40);
		for (size_t i = 0; i < s.size(); i += 2)
		{
			CHECK(s[i] == 'a');
			CHECK(s[i + 1] == 'b');
		}
		CHECK_TERMINATED(s);
	}

	// 9.4 append 之后继续 push_back
	{
		lem::MyString s("ab");
		s.append("cd");
		s.push_back('e');
		CHECK(s.size() == 5);
		CHECK_STREQ(s.c_str(), "abcde");
	}
}

#if RUN_BUG_CASES
// ---------------------------------------------------------------------------
// 10) 拷贝赋值：MyString 目前没写 operator=，编译器生成的默认版本是浅拷贝
// ---------------------------------------------------------------------------
void test_copy_assign()
{
	// 已知问题：类里写了拷贝构造和析构，却没写 operator=，
	// 编译器生成的默认拷贝赋值只是逐成员复制（_str 指针直接赋过去），
	// 于是 a 和 b 指向同一块内存，析构时 delete[] 两次 => double free
	// （ASan 报 attempting double-free）。
	// 修法：补一个深拷贝的 operator=（或用 copy-and-swap），并处理自赋值。
	lem::MyString a("aaa");
	lem::MyString b("bbb");

	a = b;
	CHECK_STREQ(a.c_str(), "bbb");
	CHECK_STREQ(b.c_str(), "bbb");
	CHECK(a.c_str() != b.c_str());   // 深拷贝：内存各自独立

	// 改 b 不能影响 a
	b[0] = 'X';
	CHECK_STREQ(a.c_str(), "bbb");

	// 自赋值：不能先 delete[] _str 再从自己拷贝（否则读到已释放内存）
	a = a;
	CHECK_STREQ(a.c_str(), "bbb");
}
#endif

// ---------------------------------------------------------------------------
// main：按顺序跑所有用例
// ---------------------------------------------------------------------------
int main()
{
	std::cout << "==================== lem::MyString tests ====================" << std::endl;

	std::cout << "[ RUN  ] test_construct" << std::endl;
	test_construct();       // 构造 / c_str / size / 隐式转换 / 长串

	std::cout << "[ RUN  ] test_copy_construct" << std::endl;
	test_copy_construct();  // 拷贝构造：深拷贝

	std::cout << "[ RUN  ] test_iterators" << std::endl;
	test_iterators();       // begin/end、const 重载、范围 for

	std::cout << "[ RUN  ] test_subscript" << std::endl;
	test_subscript();       // operator[] 读写 + '\0' 遍历

	std::cout << "[ RUN  ] test_reserve" << std::endl;
	test_reserve();         // 扩容、不缩容、预留后追加

	std::cout << "[ RUN  ] test_resize" << std::endl;
	test_resize();          // 扩大（默认/指定字符）、缩小、缩到 0

	std::cout << "[ RUN  ] test_push_back" << std::endl;
	test_push_back();       // 空串追加、多次扩容

	std::cout << "[ RUN  ] test_pop_back" << std::endl;
	test_pop_back();        // 删除、删到空、删后追加

	std::cout << "[ RUN  ] test_append" << std::endl;
	test_append();          // 追加、空串、多次扩容、混用

#if RUN_BUG_CASES
	std::cout << "[ RUN  ] test_copy_assign" << std::endl;
	test_copy_assign();     // 拷贝赋值：需要先补 operator=
#endif

	std::cout << "-------------------------------------------------------------" << std::endl;
	std::cout << "checks: " << g_total << "  pass: " << g_pass << "  fail: " << g_fail << std::endl;
	if (g_fail == 0)
	{
		std::cout << "ALL TESTS PASSED" << std::endl;
	}
	else
	{
		std::cout << "SOME TESTS FAILED -- see [FAIL] lines above" << std::endl;
	}

	return g_fail == 0 ? 0 : 1;
}

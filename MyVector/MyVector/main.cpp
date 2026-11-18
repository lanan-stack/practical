#include"MyVector.h"
#include<string>

//==============================================================
//测试开关
//下面几段测的是现在有问题的功能，代码改好之后把对应的开关改成 1 再跑：
//	TEST_COPY		拷贝构造：现在编译器用的是隐式生成的版本（浅拷贝），跑到底会二次释放
//	TEST_ASSIGN		赋值运算符：没有重载，同样是浅拷贝
//	TEST_RESIZE		resize(n)：现在编译不过，模板参数 T 推不出来
//	TEST_SELF_REF	拿自己的元素当参数：扩容后形参引用悬空，读出来是随机值
//==============================================================
#define TEST_COPY		0
#define TEST_ASSIGN		0
#define TEST_RESIZE		0
#define TEST_SELF_REF	0

//统一打印：容量和内容放一起看，方便对照
static void Show(const char* tag, lem::MyVector<int>& v)
{
	cout << tag << "  size=" << v.size() << "  cap=" << v.capacity() << "  :  ";
	v.Print();
}

//0.原来的用例，原样保留
static void TestOrigin()
{
	cout << "===== 0.原来的用例 =====" << endl;
	lem::MyVector<int> v1 = { 1,2,3 };

	v1.Print();

	v1.push_back(2026);
	v1.push_back(11);
	v1.push_back(17);

	v1.Print();

	v1.pop_back();
	v1.pop_back();
	v1.pop_back();
	v1.Print();

	v1.insert(v1.begin() + 1, 3);
	v1.Print();
	v1.erase(v1.begin());
	v1.Print();
}

//1.默认构造 / 列表构造 / empty
static void TestConstruct()
{
	cout << "===== 1.构造 =====" << endl;

	lem::MyVector<int> v0;
	Show("默认构造", v0);
	cout << "默认构造 empty()=" << v0.empty() << endl;

	lem::MyVector<int> v1 = { 1,2,3 };//vector<T> v = {1,2,3}
	Show("列表构造{1,2,3}", v1);
	cout << "列表构造 empty()=" << v1.empty() << endl;

	lem::MyVector<int> v2{ 4,5 };//不带=的写法也要能过
	Show("列表构造{4,5}", v2);
}

//2.reserve：只扩不缩，扩容后数据要保住
static void TestReserve()
{
	cout << "===== 2.reserve =====" << endl;

	lem::MyVector<int> v;
	v.reserve(10);
	Show("reserve(10)", v);//size=0 cap=10

	v.reserve(5);//比当前容量小，不应该动
	Show("reserve(5)再看", v);

	for (int i = 0; i < 3; i++)
	{
		v.push_back(i);
	}
	Show("放3个", v);

	v.reserve(20);//扩到20，原来3个数据要在
	Show("reserve(20)", v);
}

//3.push_back / pop_back：看扩容倍数
static void TestPushPop()
{
	cout << "===== 3.push_back / pop_back =====" << endl;

	lem::MyVector<int> v;
	for (int i = 1; i <= 9; i++)//cap 4->8->16
	{
		v.push_back(i * 10);
		cout << "第" << i << "次 push_back 后 cap=" << v.capacity() << endl;
	}
	Show("push_back 结束", v);

	v.pop_back();
	Show("pop_back()", v);
	v.pop_back();
	Show("再 pop_back()", v);

	//空表 pop_back 会触发 assert，调试版直接停，正常流程里别这么写
}

//4.insert / erase：头、中间、尾，以及返回值
static void TestInsertErase()
{
	cout << "===== 4.insert / erase =====" << endl;

	lem::MyVector<int> v = { 1,2,3 };
	Show("初始", v);

	v.insert(v.begin(), 0);
	Show("头插0", v);

	v.insert(v.begin() + 2, 99);
	Show("下标2处插99", v);

	v.insert(v.end(), 100);
	Show("尾插100", v);

	v.erase(v.begin());
	Show("删头", v);

	v.erase(v.end() - 1);
	Show("删尾", v);

	v.erase(v.begin() + 1);
	Show("删中间", v);

	//insert 返回的是插入位置，erase 返回的是被删位置的下一个，可以接着用
	auto it = v.insert(v.begin() + 1, 7);
	cout << "insert 返回 *it=" << *it << endl;

	it = v.erase(it);
	cout << "erase 返回 *it=" << *it << endl;
	Show("链式操作后", v);
}

//5.迭代器：range for、手写 while、改元素
static void TestIterator()
{
	cout << "===== 5.迭代器 =====" << endl;

	lem::MyVector<int> v = { 1,2,3,4,5 };

	//手写迭代器，和原来 main 里注释掉的那段一个意思
	auto vt = v.begin();
	while (vt != v.end())
	{
		cout << *vt << " ";
		++vt;
	}
	cout << endl;

	//range for 改元素要带 &，否则改的是副本
	for (auto& x : v)
	{
		x *= 2;
	}
	Show("每个元素乘2", v);

	//const 对象上只能用 const_iterator
	const lem::MyVector<int>& cv = v;
	for (auto it = cv.begin(); it != cv.end(); ++it)
	{
		cout << *it << " ";
	}
	cout << endl;
}

//6.非 int 类型：string 这种带堆内存的类也要能存能取
static void TestString()
{
	cout << "===== 6.string 类型 =====" << endl;

	lem::MyVector<std::string> vs;//这里不能用 Show，Show 只收 MyVector<int>
	vs.push_back("hello");
	vs.push_back("world");
	vs.insert(vs.begin(), "lem");
	vs.Print();

	vs.erase(vs.begin() + 1);
	vs.Print();
	cout << "size=" << vs.size() << "  cap=" << vs.capacity() << endl;
}

#if TEST_COPY
//7.拷贝构造：改完之后 b 和 a 是两段独立的空间
static void TestCopy()
{
	cout << "===== 7.拷贝构造 =====" << endl;

	lem::MyVector<int> a = { 1,2,3 };
	{
		lem::MyVector<int> b(a);
		Show("b(a)", b);

		b.push_back(4);//改 b 不能影响 a
		Show("b 加一个后 b", b);
		Show("b 加一个后 a", a);
	}
	Show("b 析构后 a", a);//现在跑到这之前就崩了
}
#endif

#if TEST_ASSIGN
//8.赋值运算符：b=a 之后两者也要各自独立
static void TestAssign()
{
	cout << "===== 8.赋值运算符 =====" << endl;

	lem::MyVector<int> a = { 1,2,3 };
	lem::MyVector<int> b = { 7,8 };

	b = a;
	Show("a", a);
	Show("b=a 后 b", b);

	b.push_back(9);
	Show("b 加一个后 a", a);//a 不能被带跑
}
#endif

#if TEST_RESIZE
//9.resize：变大补值、变小截断
static void TestResize()
{
	cout << "===== 9.resize =====" << endl;

	lem::MyVector<int> v = { 1,2,3 };
	Show("初始", v);

	v.resize(6);//变大，新位置补 T()，这里是 0
	Show("resize(6)", v);

	v.resize(10, 9);//变大补9，会触发扩容
	Show("resize(10,9)", v);

	v.resize(2);//变小
	Show("resize(2)", v);

	v.resize(0);
	Show("resize(0)", v);
}
#endif

#if TEST_SELF_REF
//10.拿自己的元素当参数：扩容之后形参引用已经悬空
static void TestSelfRef()
{
	cout << "===== 10.自身元素当参数 =====" << endl;

	lem::MyVector<int> v = { 1,2,3 };//cap=3 已经满了，push_back 会扩容
	v.push_back(*v.begin());
	Show("push_back(自己第0个)", v);

	lem::MyVector<int> w = { 10,20,30 };
	w.insert(w.begin(), *(w.begin() + 1));
	Show("insert(自己第1个)", w);
}
#endif

int main()
{
	TestOrigin();
	TestConstruct();
	TestReserve();
	TestPushPop();
	TestInsertErase();
	TestIterator();
	TestString();

#if TEST_COPY
	TestCopy();
#endif
#if TEST_ASSIGN
	TestAssign();
#endif
#if TEST_RESIZE
	TestResize();
#endif
#if TEST_SELF_REF
	TestSelfRef();
#endif

	cout << "全部测试跑完" << endl;
	return 0;
}

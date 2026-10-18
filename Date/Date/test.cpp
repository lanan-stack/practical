#include"Date.h"

int main()
{
	// 1. 默认构造：全缺省参数 2026/10/4
	Date d1;
	d1.Print();

	// 2. 加减天数：operator+ / operator- 是 const 成员，不改变原对象
	Date d2(2024, 2, 25);
	Date d3 = d2 + 5;
	d2.Print();
	d3.Print();

	// 3. 跨年进位 / 借位
	Date d4(2024, 12, 31);
	d4 += 1;
	d4.Print();

	Date d5(2024, 1, 1);
	d5 -= 1;
	d5.Print();

	// 4. 闰年判断：2000 是闰年（能被 400 整除），2100 不是（能被 100 整除但不能被 400 整除）
	Date d6(2000, 2, 28);
	d6 += 1;
	d6.Print();

	Date d7(2100, 2, 28);
	d7 += 1;
	d7.Print();

	// 5. 后置++：返回修改前的副本；前置--：返回修改后的本体
	Date d8(2024, 2, 29);
	Date d9 = d8++;
	d8.Print();
	d9.Print();

	Date d10(2024, 3, 1);
	Date d11 = --d10;
	d10.Print();
	d11.Print();

	// 6. 非法日期：CheckDate 会拒绝 2 月 30 日、4 月 31 日
	Date d12(2023, 2, 30);
	d12.Print();

	Date d13(2023, 4, 31);
	d13.Print();

	// 7. 流式输出与输入（输入格式：年 月 日，空格分隔，例如 2024 5 1）
	cout << d3 << endl;
	Date d14;
	std::cin >> d14;
	cout << d14 << endl;

	return 0;
}

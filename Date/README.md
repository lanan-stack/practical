# Date 日期类

用 C++ 实现的日期类（面向对象练习），支持日期合法性校验、按天加减、自增自减以及流式输入输出。

## 目录结构

```
Date/
├── Date.slnx                    # Visual Studio 解决方案
├── README.md                    # 本文件
├── .gitignore                   # 忽略构建产物
└── Date/
    ├── Date.h                   # Date 类声明
    ├── Date.cpp                 # Date 类实现
    ├── test.cpp                 # 测试入口（main）
    ├── Date.vcxproj             # Visual Studio 工程
    └── Date.vcxproj.filters     # 工程文件筛选器
```

## 功能

| 功能 | 接口 | 说明 |
| --- | --- | --- |
| 构造 | `Date(int year = 2026, int month = 10, int day = 4)` | 构造函数带 `explicit`，禁止 `int` 隐式转换为 `Date` |
| 拷贝构造 | `Date(const Date& d)` | 逐成员拷贝 |
| 合法性校验 | `bool CheckDate(int year, int month, int day) const` | 年 >= 0、月 1~12、日 1~当月天数 |
| 当月天数 | `static int GetMonthDay(int year, int month)` | 闰年 2 月 29 天：能被 4 整除且不能被 100 整除，或能被 400 整除 |
| 打印 | `void Print() const` | 非法日期输出 `日期非法` |
| 加减天数 | `operator+ / operator-` | `const` 成员，返回新对象，不改变原对象 |
| 累加累减 | `operator+= / operator-=` | 返回自身引用，支持负数，自动进位/借位 |
| 自增自减 | 前置 `++d` `--d`，后置 `d++` `d--` | 前置返回本体引用，后置返回修改前的副本 |
| 流输出 | `operator<<` | 输出格式 `年/月/日` |
| 流输入 | `operator>>` | 按 `年 月 日` 读取（空格分隔） |

## 编译运行

### Visual Studio

用 Visual Studio 2022 打开 `Date.slnx`，按 `F5` 编译运行。

### 命令行（Developer Command Prompt）

```bat
cl /nologo /EHsc /std:c++17 test.cpp Date.cpp /Fe:Date.exe
Date.exe
```

## 测试用例

`test.cpp` 覆盖以下场景：

1. 默认构造（2026/10/4）
2. `d + n` 不改变原对象
3. 跨年进位（2024/12/31 += 1 → 2025/1/1）与借位（2024/1/1 -= 1 → 2023/12/31）
4. 闰年判断（2000/2/28 += 1 → 2000/2/29；2100/2/28 += 1 → 2100/3/1）
5. 前置/后置自增自减
6. 非法日期（2023/2/30、2023/4/31 输出 `日期非法`）
7. `cout << d` 与 `cin >> d`

预期输出（最后一步输入 `2024 5 1`）：

```
2026/10/4
2024/2/25
2024/3/1
2025/1/1
2023/12/31
2000/2/29
2100/3/1
2024/3/1
2024/2/29
2024/2/29
2024/2/29
日期非法
日期非法
2024/3/1
2024/5/1
```

## 说明

- 源文件使用 **UTF-8 with BOM** 编码，MSVC 会自动识别，中文注释不会乱码。
- `x64/`、`*.obj`、`*.exe`、`*.pdb`、`*.ilk`、`*.tlog` 等为编译产物，已在 `.gitignore` 中忽略，克隆后本地编译即可重新生成。
- 本工程是 `D:\code\test_c` 仓库的一部分，提交范围仅限 `Date/` 目录。

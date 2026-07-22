# SListNode - 带头结点单链表实现

## 项目简介

本项目使用 **C 语言** 实现了一个**带头结点的单链表**，支持以下基本操作：

| 函数 | 功能 |
|------|------|
| `createplist()` | 创建带头结点的空链表 |
| `BuySListNode(x)` | 创建并初始化一个新节点 |
| `SLNPrint(phead)` | 打印整个链表 |
| `SLNPushFront(phead, x)` | 头插 |
| `SLNPopFront(phead)` | 头删 |
| `SLNPushBack(phead, x)` | 尾插 |
| `SLNPopBack(phead)` | 尾删 |
| `SLNFind(phead, x)` | 查找指定值节点，返回位置索引 |
| `SLNDestory(phead)` | 销毁链表，释放所有节点内存 |

## 项目结构

```
SListNode/
├── SListNode/SListNode.h   # 头文件 — 接口声明
├── SListNode/SListNode.c   # 源文件 — 链表实现
├── SListNode/test.c        # 测试文件 — 含 main
├── SListNode/SListNode.vcxproj
├── SListNode.slnx
└── README.md
```

## 设计特点

- **带头结点**：头结点为哑结点，所有操作通过头指针进行
- **内存安全**：每个函数使用 `assert` 做前置检查
- **纯 C 实现**：手动 `malloc/free` 管理内存

## 编译运行

本项目使用 **Visual Studio 2022**（MSVC v145 工具集）编译。

打开 `SListNode.slnx` 即可直接编译运行。

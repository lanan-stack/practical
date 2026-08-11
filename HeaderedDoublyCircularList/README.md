# HeaderedDoublyCircularList - 带头节点双向循环链表

## 项目简介

本项目使用 **C 语言** 实现了一个**带头节点的双向循环链表**（Headered Doubly Circular Linked List）。头节点不参与数据存储，其 `next` 指向第一个数据节点，`prev` 指向最后一个数据节点，形成首尾相接的循环结构，支持高效的头尾双向操作。

## 数据结构

```
头节点 <--> 数据节点1 <--> 数据节点2 <--> ... <--> 数据节点N <--> 头节点
```

每个节点包含：
| 字段 | 类型 | 说明 |
|------|------|------|
| `data` | `int` | 节点存储的数据 |
| `prev` | `struct DCListNode*` | 指向前驱节点 |
| `next` | `struct DCListNode*` | 指向后继节点 |
| `list_length` | `int` | 链表长度（存于头节点） |

## 接口函数

| 函数 | 功能 |
|------|------|
| `DCListInit1(DCL)**` | 初始化链表，分配头节点并指向自身 |
| `DCLPrint(DCL)` | 打印链表所有元素 |
| `DCLPushFront(DCL, x)` | 头插：在头节点后插入新节点 |
| `DCLPushBack(DCL, x)` | 尾插：在头节点前插入新节点 |
| `DCLPopFront(DCL)` | 头删：删除第一个数据节点 |
| `DCLPopBack(DCL)` | 尾删：删除最后一个数据节点 |
| `DCLGetElement(DCL, i)` | 获取第 i 个位置的数据并打印 |
| `DCLInsert(DCL, i, x)` | 在第 i 个位置后插入值为 x 的节点 |
| `DCLDelete(DCL, i)` | 删除第 i 个位置的节点 |
| `DCListDestory(DCL)**` | 销毁链表，释放所有节点内存 |

## 项目结构

```
HeaderedDoublyCircularList/
├── HeaderedDoublyCircularList.slnx          # VS 解决方案
├── HeaderedDoublyCircularList/
│   ├── DCList.h                              # 头文件 — 结构定义与接口声明
│   ├── DCList.c                              # 源文件 — 链表实现
│   ├── test.c                                # 测试文件 — 含 main
│   ├── HeaderedDoublyCircularList.vcxproj    # VS 项目配置
│   └── HeaderedDoublyCircularList.vcxproj.filters
└── README.md
```

## 编译运行

本项目使用 **Visual Studio 2022**（MSVC v145 工具集，x64 平台）编译。

打开 `HeaderedDoublyCircularList.slnx` 即可直接编译运行。

## 与单链表的区别

| 特性 | 单链表 | 双向循环链表（本项目） |
|------|--------|------------------------|
| 指针 | 仅 `next` | `prev` + `next` |
| 遍历 | 单向 | 双向 |
| 尾插效率 | O(n) | O(1) |
| 循环 | 否 | 是（头尾相接） |

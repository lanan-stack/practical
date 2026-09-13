# Circular Queue - 基于数组的循环队列实现

## 项目简介

本项目使用 **C 语言** 实现了一个**基于数组的循环队列**（Circular Queue）。利用取模运算在固定大小的数组上实现环形访问，**多浪费一个空间**区分队满和队空：

- **队空**：`front == rear`
- **队满**：`(rear + 1) % N == front`

## 接口函数

| 函数 | 功能 |
|------|------|
| `circular_queue_create(k)` | 创建容量为 k 的循环队列 |
| `circular_queue_destroy` | 销毁队列，释放内存 |
| `circular_queue_isfull` | 判断队列是否已满 |
| `circular_queue_isempty` | 判断队列是否为空 |
| `circular_queue_push` | 入队 |
| `circular_queue_pop` | 出队 |
| `circular_queue_front` | 查看队头元素 |
| `circular_queue_rear` | 查看队尾元素 |

## 项目结构

```
Circular Queue/
├── Circular Queue/
│   ├── circularqueue.h          # 头文件 — 结构定义与接口声明
│   ├── circularqueue.c          # 源文件 — 循环队列实现
│   ├── test.c                   # 测试文件 — 含 main
│   ├── Circular Queue.vcxproj
│   └── Circular Queue.vcxproj.filters
└── README.md
```

## 编译运行

使用 **Visual Studio 2022**（MSVC v145 工具集，x64 平台）。

打开 `Circular Queue.slnx` 即可直接编译运行。

## 操作演示

```
创建容量为 5 的队列（可用空间 4）
入队: 1 → 2 → 3 → 4（满）
出队: 1
队列: [空] 2 → 3 → 4 → [尾]
```

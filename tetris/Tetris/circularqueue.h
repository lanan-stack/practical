#pragma once
// ============================================================
// circularqueue.h —— 循环队列（项目原有代码）
// ------------------------------------------------------------
// 【Phase 4 补充说明】
// 1. 顶上加了 #pragma once：防止同一个文件里重复包含导致结构体重复定义。
//    这是新增的保护，结构体与原有函数的定义一字未改。
// 2. 文件末尾新增三个函数声明，是袋子系统需要的新能力：
//    查元素个数、看第 N 个、出队并取值。原有八个函数保持原样。
// ============================================================

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>

typedef int CQDataType;

typedef struct
{
	int* a;
	int front;
	int rear;
	int N;//数组空间
}circular_queue;

circular_queue* circular_queue_create(int k);//初始化
void circular_queue_destroy(circular_queue* obj);//销毁
bool circular_queue_isfull(circular_queue* obj);//判满
bool circular_queue_isempty(circular_queue* obj);//判空
bool circular_queue_push(circular_queue* obj, int x);//入队
bool circular_queue_pop(circular_queue* obj);//出队
int circular_queue_front(circular_queue* obj); //取队头
int circular_queue_rear(circular_queue* obj);//取队尾
// ============================================================
// 以下三个是 Phase 4 新增的能力（原有函数一字未改）
// ------------------------------------------------------------
// 为什么需要它们：袋子系统要"预览队头后面 3 个是什么"和"取走队头并拿到它的值"，
// 而原有接口只有"看队头"和"出队但不返回值"，不够用。
//
// 契约风格：新函数一律"返回 bool 表示成功与否 + 出参带回数据"，
// 不使用 -1 这类魔法值表示失败（老函数保持原来的 -1 约定不变）。
// 另外沿用本文件既有风格：用 assert 检查空指针，传 NULL 属于调用方违约。
// ============================================================

// 取当前元素个数
int circular_queue_size(const circular_queue* obj);

// 查看从队头数第 index 个元素（index = 0 表示队头）
// 返回：true = 成功并把值写入 *outValue；false = 队列为空或 index 越界（此时不改动队列）
bool circular_queue_peek_at(const circular_queue* obj, int index, int* outValue);

// 出队并把被移走的元素值写入 *outValue
// 返回：true = 成功；false = 队列为空或 outValue 为 NULL（此时队列不变）
bool circular_queue_pop_value(circular_queue* obj, int* outValue);

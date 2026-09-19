// ============================================================
// test_placeholder.cpp —— Phase 0 的占位测试
// ------------------------------------------------------------
// 所属层次：测试工程
//
// 【本阶段目标】
// 只证明三件事：
//   1. gtest 框架能编译、能链接、能运行；
//   2. 游戏工程里的循环队列源码能被测试工程正确编译并链接；
//   3. Visual Studio 的测试资源管理器能自动发现这些用例。
//
// 【本阶段刻意不做的事】
// 循环队列的正式功能测试（预览第 N 个、出队取值等）留到 Phase 4，
// 形状与旋转的测试留到 Phase 2。
// ============================================================

#include <gtest/gtest.h>

extern "C" {
#include "circularqueue.h"   // 游戏工程里的循环队列（测试工程直接引用其源码）
}   // 游戏工程里的循环队列（测试工程直接引用其源码）

// ---------- 用例 1：证明 gtest 本身工作正常 ----------
TEST(Phase0, GtestWorks)
{
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

// ---------- 用例 2：工程联通性冒烟测试 ----------
// 这条用例不是"循环队列的功能测试"，而是一次工程检查：
// 证明测试工程能找到、能编译、能链接游戏工程里的 C 源码。
// 如果链接配置有问题（比如没把 circularqueue.c 加进测试工程），
// 这里会直接编译失败，而不是等到 Phase 4 才发现。
TEST(Phase0, CircularQueueLinks)
{
    // 建一个能装 7 个元素的队列（7 正好是一袋方块的数量）
    circular_queue* q = circular_queue_create(7);
    ASSERT_TRUE(q != NULL);                 // 创建成功

    EXPECT_TRUE(circular_queue_isempty(q)); // 新建出来的队列应该是空的

    // 入队 1 个元素后，应变成非空
    EXPECT_TRUE(circular_queue_push(q, 3));
    EXPECT_FALSE(circular_queue_isempty(q));
    EXPECT_EQ(circular_queue_front(q), 3);

    // 用完释放，避免内存泄漏
    circular_queue_destroy(q);
}

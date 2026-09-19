// ============================================================
// test_circularqueue.cpp —— Phase 4 单元测试：循环队列（含新增能力）
// ------------------------------------------------------------
// 对应用例 P4-01 ~ P4-07（清单已由用户审核通过）。
//
// 被测代码：circularqueue.h / circularqueue.c（项目原有代码 + Phase 4 新增三个函数）
// 特别关注：**原有函数的行为必须与扩展前完全一致**（回归测试），
//           新增的三个函数在边界上必须安全（空队列、越界、容量 0/1）。
// ============================================================

#include <gtest/gtest.h>

extern "C" {
#include "circularqueue.h"
}

// ============================================================
// P4-01 原有函数回归：判空、判满、入队、出队、取队头、取队尾
// ============================================================
TEST(Phase4Queue, P4_01_OriginalFunctionsStillWork)
{
    circular_queue* q = circular_queue_create(3);
    ASSERT_NE(q, nullptr);

    // 新建的队列：空但不满，取队头/队尾返回 -1，出队失败
    EXPECT_TRUE(circular_queue_isempty(q));
    EXPECT_FALSE(circular_queue_isfull(q));
    EXPECT_EQ(circular_queue_front(q), -1);
    EXPECT_EQ(circular_queue_rear(q), -1);
    EXPECT_FALSE(circular_queue_pop(q));

    // 装 3 个（容量就是 3）
    EXPECT_TRUE(circular_queue_push(q, 10));
    EXPECT_TRUE(circular_queue_push(q, 20));
    EXPECT_TRUE(circular_queue_push(q, 30));

    // 满了：再入队必须失败；队头队尾取值正确
    EXPECT_TRUE(circular_queue_isfull(q));
    EXPECT_FALSE(circular_queue_push(q, 40));
    EXPECT_EQ(circular_queue_front(q), 10);
    EXPECT_EQ(circular_queue_rear(q), 30);

    circular_queue_destroy(q);
}

// ============================================================
// P4-02 元素个数的变化
// ============================================================
TEST(Phase4Queue, P4_02_SizeChangesCorrectly)
{
    circular_queue* q = circular_queue_create(3);
    ASSERT_NE(q, nullptr);

    EXPECT_EQ(circular_queue_size(q), 0);

    circular_queue_push(q, 1);
    circular_queue_push(q, 2);
    circular_queue_push(q, 3);
    EXPECT_EQ(circular_queue_size(q), 3);

    EXPECT_TRUE(circular_queue_pop(q));
    EXPECT_EQ(circular_queue_size(q), 2);

    EXPECT_TRUE(circular_queue_pop(q));
    EXPECT_TRUE(circular_queue_pop(q));
    EXPECT_EQ(circular_queue_size(q), 0);
    EXPECT_TRUE(circular_queue_isempty(q));

    circular_queue_destroy(q);
}

// ============================================================
// P4-03 查看第 N 个（含队头绕过一圈之后的情形）
// ============================================================
TEST(Phase4Queue, P4_03_PeekAtIndex)
{
    circular_queue* q = circular_queue_create(3);
    ASSERT_NE(q, nullptr);

    // 先装满再出队一个、再入队一个，让队头"绕过一圈"（front 不再等于 0）
    circular_queue_push(q, 1);
    circular_queue_push(q, 2);
    circular_queue_push(q, 3);
    EXPECT_TRUE(circular_queue_pop(q));      // 移走 1，队头前进
    EXPECT_TRUE(circular_queue_push(q, 4));  // 此时队列里是 2, 3, 4

    int v = -1;
    ASSERT_TRUE(circular_queue_peek_at(q, 0, &v));
    EXPECT_EQ(v, 2) << "第 0 个应该是队头";
    ASSERT_TRUE(circular_queue_peek_at(q, 1, &v));
    EXPECT_EQ(v, 3);
    ASSERT_TRUE(circular_queue_peek_at(q, 2, &v));
    EXPECT_EQ(v, 4);

    // 越界：负数、等于元素个数，都必须失败且不改动队列
    EXPECT_FALSE(circular_queue_peek_at(q, -1, &v));
    EXPECT_FALSE(circular_queue_peek_at(q, 3, &v));
    EXPECT_EQ(circular_queue_size(q), 3);
    ASSERT_TRUE(circular_queue_peek_at(q, 0, &v));
    EXPECT_EQ(v, 2) << "越界读取之后队列内容不应被改动";

    // 空队列上任何下标都失败
    circular_queue_pop(q);
    circular_queue_pop(q);
    circular_queue_pop(q);
    EXPECT_TRUE(circular_queue_isempty(q));
    EXPECT_FALSE(circular_queue_peek_at(q, 0, &v));

    circular_queue_destroy(q);
}

// ============================================================
// P4-04 出队并取值（含只剩一个、空队列）
// ============================================================
TEST(Phase4Queue, P4_04_PopValue)
{
    circular_queue* q = circular_queue_create(2);
    ASSERT_NE(q, nullptr);

    circular_queue_push(q, 7);
    circular_queue_push(q, 8);

    int v = -1;
    ASSERT_TRUE(circular_queue_pop_value(q, &v));
    EXPECT_EQ(v, 7);
    EXPECT_EQ(circular_queue_size(q), 1);

    // 只剩一个也要能正确取出
    ASSERT_TRUE(circular_queue_pop_value(q, &v));
    EXPECT_EQ(v, 8);
    EXPECT_EQ(circular_queue_size(q), 0);

    // 空队列：返回 false，且不改动任何状态
    v = 12345;
    EXPECT_FALSE(circular_queue_pop_value(q, &v));
    EXPECT_EQ(v, 12345) << "失败时不应写入 outValue";
    EXPECT_EQ(circular_queue_size(q), 0);

    circular_queue_destroy(q);
}

// ============================================================
// P4-05 环形复用：下标绕回后顺序仍然正确
// ============================================================
TEST(Phase4Queue, P4_05_WraparoundKeepsOrder)
{
    circular_queue* q = circular_queue_create(3);
    ASSERT_NE(q, nullptr);

    for (int round = 0; round < 100; round++)
    {
        const int base = round * 10;
        ASSERT_TRUE(circular_queue_push(q, base + 1));
        ASSERT_TRUE(circular_queue_push(q, base + 2));
        ASSERT_TRUE(circular_queue_push(q, base + 3));
        EXPECT_EQ(circular_queue_size(q), 3);
        EXPECT_FALSE(circular_queue_push(q, -999)) << "第 " << round << " 轮：满了还能入队？";

        for (int k = 1; k <= 3; k++)
        {
            int v = 0;
            ASSERT_TRUE(circular_queue_pop_value(q, &v));
            EXPECT_EQ(v, base + k) << "第 " << round << " 轮第 " << k << " 个顺序不对";
        }
        EXPECT_EQ(circular_queue_size(q), 0);
    }

    circular_queue_destroy(q);
}

// ============================================================
// P4-06 新旧函数混用：pop（不返回值）与 pop_value 交替使用
// ============================================================
TEST(Phase4Queue, P4_06_OldAndNewPopCanBeMixed)
{
    circular_queue* q = circular_queue_create(4);
    ASSERT_NE(q, nullptr);

    circular_queue_push(q, 1);
    circular_queue_push(q, 2);
    circular_queue_push(q, 3);
    circular_queue_push(q, 4);

    // 用老的 pop 移走 1
    EXPECT_TRUE(circular_queue_pop(q));
    EXPECT_EQ(circular_queue_size(q), 3);

    // 用新的 pop_value 取 2、3
    int v = 0;
    ASSERT_TRUE(circular_queue_pop_value(q, &v));
    EXPECT_EQ(v, 2);
    ASSERT_TRUE(circular_queue_pop_value(q, &v));
    EXPECT_EQ(v, 3);

    // 再入队一个，此时队列里是 4, 5
    EXPECT_TRUE(circular_queue_push(q, 5));
    EXPECT_TRUE(circular_queue_pop(q));            // 移走 4
    ASSERT_TRUE(circular_queue_pop_value(q, &v));  // 取 5
    EXPECT_EQ(v, 5);
    EXPECT_EQ(circular_queue_size(q), 0);

    circular_queue_destroy(q);
}

// ============================================================
// P4-07 容量边界：create(0) 与 create(1)
// ------------------------------------------------------------
// 本队列用"牺牲一个存储单元"的经典写法区分空与满，所以：
//   create(0) → 一个元素也存不下，永远既空又满，入队必然失败；
//   create(1) → 恰好能存 1 个。
// 这两条不是 bug，而是该数据结构的既定行为，用测试把它钉住。
// ============================================================
TEST(Phase4Queue, P4_07_ZeroAndOneCapacity)
{
    // ---------- 容量 0 ----------
    circular_queue* q0 = circular_queue_create(0);
    ASSERT_NE(q0, nullptr);
    EXPECT_TRUE(circular_queue_isempty(q0));
    EXPECT_EQ(circular_queue_size(q0), 0);
    EXPECT_FALSE(circular_queue_push(q0, 1)) << "容量 0 的队列不该能入队";
    EXPECT_TRUE(circular_queue_isempty(q0));
    circular_queue_destroy(q0);

    // ---------- 容量 1 ----------
    circular_queue* q1 = circular_queue_create(1);
    ASSERT_NE(q1, nullptr);
    EXPECT_EQ(circular_queue_size(q1), 0);
    EXPECT_TRUE(circular_queue_push(q1, 42));
    EXPECT_EQ(circular_queue_size(q1), 1);
    EXPECT_TRUE(circular_queue_isfull(q1));
    EXPECT_FALSE(circular_queue_push(q1, 43)) << "容量 1 的队列存满后不该能再入队";

    int v = 0;
    EXPECT_TRUE(circular_queue_pop_value(q1, &v));
    EXPECT_EQ(v, 42);
    EXPECT_EQ(circular_queue_size(q1), 0);
    circular_queue_destroy(q1);
}

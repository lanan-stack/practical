// ============================================================
// test_hold.cpp —— Phase 4 单元测试：暂存（Hold）规则
// ------------------------------------------------------------
// 对应用例 P4-24 ~ P4-29（清单已由用户审核通过）。
//
// 被测代码：hold.h / hold.c（依赖 bag 与循环队列）
//
// 说明：本轮只验证"编号互换"与"每个方块只能换一次"这两类规则；
//       "换出来的方块旋转姿态归零"属于游戏状态，留到 Phase 6 验证。
// ============================================================

#include <gtest/gtest.h>

#include "hold.h"
#include "bag.h"
#include "config.h"

// ============================================================
// P4-24 槽为空时暂存：当前块进槽，新当前块 = 原预览第 0 个，队列少一个
// ============================================================
TEST(Phase4Hold, P4_24_HoldWithEmptySlot)
{
    Bag bag;
    bag_init(&bag, 20240919);
    Hold hold;
    hold_init(&hold);

    // 模拟"场上已经有一块"：先从袋子里取一块作为当前方块
    int current = -1;
    ASSERT_TRUE(bag_next(&bag, &current));
    const int savedCurrent = current;

    // 记下此刻的预览第 0 个（也就是下一次要出场的方块）
    int expectedNext = -1;
    ASSERT_TRUE(bag_peek(&bag, 0, &expectedNext));
    const int sizeBefore = bag_queue_size(&bag);

    ASSERT_TRUE(hold_apply(&hold, &bag, &current));

    EXPECT_TRUE(hold.hasPiece);
    EXPECT_EQ(hold.slotId, savedCurrent) << "原来的当前方块应该被放进槽里";
    EXPECT_EQ(current, expectedNext)     << "新的当前方块应该是原预览第 0 个";
    EXPECT_EQ(bag_queue_size(&bag), sizeBefore - 1) << "槽为空时暂存会从袋子里取走一块";

    bag_destroy(&bag);
}

// ============================================================
// P4-25 槽非空时互换：两块互换，队列不变，预览不变
// ============================================================
TEST(Phase4Hold, P4_25_HoldSwapsWhenSlotNotEmpty)
{
    Bag bag;
    bag_init(&bag, 8888);
    Hold hold;
    hold_init(&hold);

    int current = -1;
    ASSERT_TRUE(bag_next(&bag, &current));

    // 第一次暂存：槽变非空
    ASSERT_TRUE(hold_apply(&hold, &bag, &current));
    hold_mark_locked(&hold);   // 模拟该方块已落地锁定

    const int currentBefore = current;
    const int slotBefore = hold.slotId;
    const int sizeBefore = bag_queue_size(&bag);

    int previewBefore[3] = { -1, -1, -1 };
    for (int i = 0; i < 3; i++)
    {
        ASSERT_TRUE(bag_peek(&bag, i, &previewBefore[i]));
    }

    ASSERT_TRUE(hold_apply(&hold, &bag, &current));

    EXPECT_EQ(current, slotBefore)   << "换出来的应该是原来槽里的那块";
    EXPECT_EQ(hold.slotId, currentBefore) << "槽里应该是换进去的那块";
    EXPECT_EQ(bag_queue_size(&bag), sizeBefore) << "槽非空时互换不该动袋子";

    for (int i = 0; i < 3; i++)
    {
        int now = -1;
        ASSERT_TRUE(bag_peek(&bag, i, &now));
        EXPECT_EQ(now, previewBefore[i]) << "互换不该影响预览";
    }

    bag_destroy(&bag);
}

// ============================================================
// P4-26 每个方块只能换一次：连续两次调用，第二次无效
// ============================================================
TEST(Phase4Hold, P4_26_OnlyOncePerPiece)
{
    Bag bag;
    bag_init(&bag, 555);
    Hold hold;
    hold_init(&hold);

    int current = -1;
    ASSERT_TRUE(bag_next(&bag, &current));

    ASSERT_TRUE(hold_apply(&hold, &bag, &current));

    // 记下状态，第二次调用必须什么都不改
    const int currentAfterFirst = current;
    const int slotAfterFirst = hold.slotId;
    const int sizeAfterFirst = bag_queue_size(&bag);

    EXPECT_FALSE(hold_apply(&hold, &bag, &current)) << "同一块不允许换第二次";
    EXPECT_EQ(current, currentAfterFirst);
    EXPECT_EQ(hold.slotId, slotAfterFirst);
    EXPECT_EQ(bag_queue_size(&bag), sizeAfterFirst);

    bag_destroy(&bag);
}

// ============================================================
// P4-27 方块锁定后解锁，可以再次暂存
// ============================================================
TEST(Phase4Hold, P4_27_UnlockedAfterLock)
{
    Bag bag;
    bag_init(&bag, 4321);
    Hold hold;
    hold_init(&hold);

    int current = -1;
    ASSERT_TRUE(bag_next(&bag, &current));

    ASSERT_TRUE(hold_apply(&hold, &bag, &current));
    EXPECT_FALSE(hold_apply(&hold, &bag, &current));   // 已经被锁住

    hold_mark_locked(&hold);                           // 方块落地
    EXPECT_TRUE(hold_apply(&hold, &bag, &current)) << "锁定之后应该又能暂存";

    bag_destroy(&bag);
}

// ============================================================
// P4-28 连续三次暂存的轮换关系
// ------------------------------------------------------------
// 设开局当前块为 A，第一次暂存后槽=A、当前块=B；
// 第二次暂存（先标记锁定）后槽=B、当前块=A；
// 第三次暂存后槽=A、当前块=B。
// 这条能抓住"槽内容被覆盖"或"当前块没更新"之类的错误。
// ============================================================
TEST(Phase4Hold, P4_28_RepeatedHoldsRotateCorrectly)
{
    Bag bag;
    bag_init(&bag, 31415);
    Hold hold;
    hold_init(&hold);

    int current = -1;
    ASSERT_TRUE(bag_next(&bag, &current));
    const int pieceA = current;

    // 第一次：槽空 → A 进槽，取到 B 作为当前块
    ASSERT_TRUE(hold_apply(&hold, &bag, &current));
    const int pieceB = current;
    EXPECT_EQ(hold.slotId, pieceA);
    EXPECT_NE(pieceB, pieceA);   // 同一袋内不会连着出同一块

    // 第二次：互换 → 当前块变回 A，槽变 B
    hold_mark_locked(&hold);
    ASSERT_TRUE(hold_apply(&hold, &bag, &current));
    EXPECT_EQ(current, pieceA);
    EXPECT_EQ(hold.slotId, pieceB);

    // 第三次：互换 → 当前块变回 B，槽变 A
    hold_mark_locked(&hold);
    ASSERT_TRUE(hold_apply(&hold, &bag, &current));
    EXPECT_EQ(current, pieceB);
    EXPECT_EQ(hold.slotId, pieceA);

    bag_destroy(&bag);
}

// ============================================================
// P4-29 混合操作不破坏袋子
// ------------------------------------------------------------
// 每三步尝试一次暂存，其余步取新方块；每步都检查：
//   · 队列元素数始终在合法范围（≥ 4 且 ≤ 容量）；
//   · 预览与出场始终一致；
//   · 暂存成功时，当前块的去向符合规则。
// ============================================================
TEST(Phase4Hold, P4_29_MixedOperationsKeepBagHealthy)
{
    Bag bag;
    bag_init(&bag, 5150);
    Hold hold;
    hold_init(&hold);

    int current = -1;
    ASSERT_TRUE(bag_next(&bag, &current));

    for (int step = 0; step < 200; step++)
    {
        int previewed = -1;
        ASSERT_TRUE(bag_peek(&bag, 0, &previewed));

        const bool slotWasEmpty = !hold.hasPiece;
        const int slotBefore = hold.slotId;
        const int currentBefore = current;

        if (step % 3 == 0 && hold_apply(&hold, &bag, &current))
        {
            // 暂存成功：槽为空时新当前块是原预览第 0 个；槽非空时是原槽内容
            if (slotWasEmpty)
            {
                EXPECT_EQ(current, previewed) << "第 " << step << " 步：新当前块不是预览第 0 个";
                EXPECT_EQ(hold.slotId, currentBefore);
            }
            else
            {
                EXPECT_EQ(current, slotBefore);
                EXPECT_EQ(hold.slotId, currentBefore);
            }
        }
        else
        {
            // 取新方块：必须等于刚才预览到的那个
            int taken = -1;
            ASSERT_TRUE(bag_next(&bag, &taken));
            EXPECT_EQ(taken, previewed) << "第 " << step << " 步：出场方块与预览不一致";
            current = taken;
            hold_mark_locked(&hold);   // 相当于上一块落地
        }

        const int size = bag_queue_size(&bag);
        EXPECT_GE(size, BAG_REFILL_THRESHOLD + 1) << "第 " << step << " 步：队列元素太少";
        EXPECT_LE(size, BAG_QUEUE_CAPACITY) << "第 " << step << " 步：队列溢出";

        EXPECT_GE(current, 0);
        EXPECT_LT(current, TETROMINO_COUNT);
    }

    bag_destroy(&bag);
}

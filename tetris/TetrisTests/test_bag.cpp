// ============================================================
// test_bag.cpp —— Phase 4 单元测试：随机数、洗牌、7-bag 袋子
// ------------------------------------------------------------
// 对应用例 P4-08 ~ P4-23（清单已由用户审核通过）。
//
// 被测代码：bag.h / bag.c（依赖项目原有的循环队列）
// ============================================================

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

#include "bag.h"
#include "config.h"

// ============================================================
// 测试辅助
// ============================================================

// 连续取 n 个方块编号；中途取失败就提前结束（正常不会发生）
static std::vector<int> TakeMany(Bag* bag, int n)
{
    std::vector<int> out;
    for (int i = 0; i < n; i++)
    {
        int id = -1;
        if (!bag_next(bag, &id))
        {
            break;
        }
        out.push_back(id);
    }
    return out;
}

// 校验从 offset 开始的 7 个方块构成完整的一袋（七种各一次）
static void ExpectOneFullBag(const std::vector<int>& pieces, int offset, const char* what)
{
    ASSERT_GE((int)pieces.size(), offset + TETROMINO_COUNT);

    int count[TETROMINO_COUNT] = { 0 };
    for (int i = 0; i < TETROMINO_COUNT; i++)
    {
        const int id = pieces[offset + i];
        ASSERT_GE(id, 0) << what << "：出现了非法编号";
        ASSERT_LT(id, TETROMINO_COUNT) << what << "：出现了非法编号 " << id;
        count[id]++;
    }

    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        EXPECT_EQ(count[id], 1) << what << "：编号 " << id << " 出现了 " << count[id] << " 次";
    }
}

// ============================================================
// B 组：随机数与洗牌
// ============================================================

// P4-08 同种子可复现
TEST(Phase4Rng, P4_08_SameSeedSameSequence)
{
    unsigned int s1 = 12345;
    unsigned int s2 = 12345;
    for (int i = 0; i < 20; i++)
    {
        EXPECT_EQ(bag_rng_next(&s1), bag_rng_next(&s2)) << "第 " << i << " 个随机数不同";
    }
}

// P4-09 不同种子应产生不同的序列
TEST(Phase4Rng, P4_09_DifferentSeedsDiffer)
{
    unsigned int a = 1;
    unsigned int b = 2;
    bool anyDifferent = false;
    for (int i = 0; i < 20; i++)
    {
        if (bag_rng_next(&a) != bag_rng_next(&b))
        {
            anyDifferent = true;
        }
    }
    EXPECT_TRUE(anyDifferent);
}

// P4-10 种子 0 不能退化成"永远输出 0"
// xorshift 类算法的经典陷阱：状态为 0 时永远输出 0，洗牌会彻底失效。
TEST(Phase4Rng, P4_10_ZeroSeedDoesNotDegenerate)
{
    unsigned int s = 0;
    std::vector<unsigned int> values;
    for (int i = 0; i < 20; i++)
    {
        values.push_back(bag_rng_next(&s));
    }

    const bool anyNonZero = std::any_of(values.begin(), values.end(),
                                        [](unsigned int v) { return v != 0; });
    EXPECT_TRUE(anyNonZero) << "种子 0 时随机数全是 0，洗牌会失效";

    std::set<unsigned int> distinct(values.begin(), values.end());
    EXPECT_GT(distinct.size(), 1u) << "种子 0 时随机数全都一样";
}

// P4-11 洗牌合法性：结果必须是 0~6 的一种排列
TEST(Phase4Rng, P4_11_ShuffleProducesPermutation)
{
    for (int seed = 1; seed <= 50; seed++)
    {
        unsigned int state = (unsigned int)seed;
        int ids[TETROMINO_COUNT];
        for (int i = 0; i < TETROMINO_COUNT; i++)
        {
            ids[i] = i;
        }

        bag_shuffle(ids, TETROMINO_COUNT, &state);

        std::vector<int> sorted(ids, ids + TETROMINO_COUNT);
        std::sort(sorted.begin(), sorted.end());
        for (int i = 0; i < TETROMINO_COUNT; i++)
        {
            EXPECT_EQ(sorted[i], i) << "种子 " << seed << " 洗牌后有重复或缺失";
        }
    }
}

// P4-12 洗牌确实在打乱：200 个种子的首位应覆盖全部 7 种编号
TEST(Phase4Rng, P4_12_ShuffleActuallyShuffles)
{
    bool seenFirst[TETROMINO_COUNT] = { false };

    for (int seed = 1; seed <= 200; seed++)
    {
        unsigned int state = (unsigned int)seed;
        int ids[TETROMINO_COUNT];
        for (int i = 0; i < TETROMINO_COUNT; i++)
        {
            ids[i] = i;
        }

        bag_shuffle(ids, TETROMINO_COUNT, &state);
        seenFirst[ids[0]] = true;
    }

    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        EXPECT_TRUE(seenFirst[id]) << "编号 " << id << " 从来没有出现在首位，洗牌可能退化了";
    }
}

// ============================================================
// C 组：7-bag 袋子
// ============================================================

// P4-13 初始化：一袋 7 个入队，预览 3 个能取到且互不相同
TEST(Phase4Bag, P4_13_InitLoadsOneBag)
{
    Bag bag;
    bag_init(&bag, 777);

    EXPECT_EQ(bag_queue_size(&bag), TETROMINO_COUNT) << "开局队列里应该正好是一袋（7 个）";

    int p0 = -1, p1 = -1, p2 = -1;
    ASSERT_TRUE(bag_peek(&bag, 0, &p0));
    ASSERT_TRUE(bag_peek(&bag, 1, &p1));
    ASSERT_TRUE(bag_peek(&bag, 2, &p2));

    for (int id : { p0, p1, p2 })
    {
        EXPECT_GE(id, 0);
        EXPECT_LT(id, TETROMINO_COUNT);
    }
    EXPECT_NE(p0, p1) << "同一袋里前三个不可能相同";
    EXPECT_NE(p1, p2);
    EXPECT_NE(p0, p2);

    bag_destroy(&bag);
}

// P4-14 第一袋完整性：连续取 7 个，七种各一次
TEST(Phase4Bag, P4_14_FirstBagIsComplete)
{
    Bag bag;
    bag_init(&bag, 20240919);

    const std::vector<int> pieces = TakeMany(&bag, TETROMINO_COUNT);
    ASSERT_EQ((int)pieces.size(), TETROMINO_COUNT);
    ExpectOneFullBag(pieces, 0, "第一袋");

    bag_destroy(&bag);
}

// P4-15 两袋各自完整：连续取 14 个，前 7 与后 7 各自都是完整排列
TEST(Phase4Bag, P4_15_TwoBagsAreEachComplete)
{
    Bag bag;
    bag_init(&bag, 5566);

    const std::vector<int> pieces = TakeMany(&bag, TETROMINO_COUNT * 2);
    ASSERT_EQ((int)pieces.size(), TETROMINO_COUNT * 2);
    ExpectOneFullBag(pieces, 0, "第一袋");
    ExpectOneFullBag(pieces, TETROMINO_COUNT, "第二袋");

    bag_destroy(&bag);
}

// P4-16 预览与出场一致：先看预览第 0 个，再取块，两者必须相同
TEST(Phase4Bag, P4_16_PreviewMatchesActualNextPiece)
{
    Bag bag;
    bag_init(&bag, 31415);

    for (int i = 0; i < 20; i++)
    {
        int peeked = -1;
        int taken = -1;
        ASSERT_TRUE(bag_peek(&bag, 0, &peeked));
        ASSERT_TRUE(bag_next(&bag, &taken));
        EXPECT_EQ(taken, peeked) << "第 " << i << " 次：预览与出场不一致";
    }

    bag_destroy(&bag);
}

// P4-17 预览窗口前移不变式（并确认覆盖到了补袋路径）
// 取块后：新预览[0] 必须等于旧预览[1]，新预览[1] 必须等于旧预览[2]。
// 由于补袋只往队尾追加，这个关系在任何时候都成立。
TEST(Phase4Bag, P4_17_PreviewWindowSlidesForward)
{
    Bag bag;
    bag_init(&bag, 2024);

    int prev[3] = { -1, -1, -1 };
    for (int i = 0; i < 3; i++)
    {
        ASSERT_TRUE(bag_peek(&bag, i, &prev[i]));
    }

    int refillCount = 0;
    for (int step = 0; step < 100; step++)
    {
        const int sizeBefore = bag_queue_size(&bag);

        int taken = -1;
        ASSERT_TRUE(bag_next(&bag, &taken));
        EXPECT_EQ(taken, prev[0]) << "第 " << step << " 步：出场方块不是预览第 0 个";

        if (bag_queue_size(&bag) > sizeBefore)
        {
            refillCount++;
        }

        int now[3] = { -1, -1, -1 };
        for (int k = 0; k < 3; k++)
        {
            ASSERT_TRUE(bag_peek(&bag, k, &now[k]));
        }
        EXPECT_EQ(now[0], prev[1]) << "第 " << step << " 步：预览没有整体前移";
        EXPECT_EQ(now[1], prev[2]) << "第 " << step << " 步：预览没有整体前移";

        for (int k = 0; k < 3; k++)
        {
            prev[k] = now[k];
        }
    }

    EXPECT_GT(refillCount, 0)
        << "这 100 步里没有发生补袋，说明上面这些断言没覆盖到补袋路径";

    bag_destroy(&bag);
}

// P4-18 队列不空也不溢出
TEST(Phase4Bag, P4_18_QueueNeverEmptyNorOverflow)
{
    Bag bag;
    bag_init(&bag, 8675309);

    for (int i = 0; i < 1000; i++)
    {
        int id = -1;
        ASSERT_TRUE(bag_next(&bag, &id)) << "第 " << i << " 次取块失败";
        ASSERT_GE(id, 0);
        ASSERT_LT(id, TETROMINO_COUNT);

        const int size = bag_queue_size(&bag);
        ASSERT_GE(size, BAG_REFILL_THRESHOLD + 1)
            << "第 " << i << " 次取块后队列只剩 " << size << " 个（补袋没生效？）";
        ASSERT_LE(size, BAG_QUEUE_CAPACITY)
            << "第 " << i << " 次取块后队列有 " << size << " 个，超出容量";
    }

    bag_destroy(&bag);
}

// P4-19 预览下标越界必须安全失败
TEST(Phase4Bag, P4_19_PeekRejectsOutOfRangeIndex)
{
    Bag bag;
    bag_init(&bag, 4242);

    int id = 12345;
    EXPECT_FALSE(bag_peek(&bag, -1, &id));
    EXPECT_FALSE(bag_peek(&bag, BAG_PREVIEW_COUNT, &id));
    EXPECT_FALSE(bag_peek(&bag, 100, &id));
    EXPECT_EQ(id, 12345) << "越界失败时不应写入 outId";

    bag_destroy(&bag);
}

// P4-20 袋子可复现：同种子的两个袋子，序列必须完全相同
TEST(Phase4Bag, P4_20_BagIsReproducibleBySeed)
{
    Bag a;
    Bag b;
    bag_init(&a, 13579);
    bag_init(&b, 13579);

    for (int i = 0; i < 50; i++)
    {
        int ida = -1, idb = -1;
        ASSERT_TRUE(bag_next(&a, &ida));
        ASSERT_TRUE(bag_next(&b, &idb));
        EXPECT_EQ(ida, idb) << "第 " << i << " 个方块不同";
    }

    bag_destroy(&a);
    bag_destroy(&b);
}

// P4-21 不同种子的序列不应完全相同
TEST(Phase4Bag, P4_21_DifferentSeedsProduceDifferentSequences)
{
    Bag a;
    Bag b;
    bag_init(&a, 24680);
    bag_init(&b, 13579);

    bool anyDifferent = false;
    for (int i = 0; i < 50; i++)
    {
        int ida = -1, idb = -1;
        ASSERT_TRUE(bag_next(&a, &ida));
        ASSERT_TRUE(bag_next(&b, &idb));
        if (ida != idb)
        {
            anyDifferent = true;
        }
    }
    EXPECT_TRUE(anyDifferent);

    bag_destroy(&a);
    bag_destroy(&b);
}

// P4-22 长期均匀：700 个 = 100 袋，每种编号恰好 100 次
TEST(Phase4Bag, P4_22_LongRunIsPerfectlyUniform)
{
    Bag bag;
    bag_init(&bag, 99991);

    const int total = TETROMINO_COUNT * 100;   // 700
    const std::vector<int> pieces = TakeMany(&bag, total);
    ASSERT_EQ((int)pieces.size(), total);

    int count[TETROMINO_COUNT] = { 0 };
    for (int id : pieces)
    {
        ASSERT_GE(id, 0);
        ASSERT_LT(id, TETROMINO_COUNT);
        count[id]++;
    }
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        EXPECT_EQ(count[id], 100) << "编号 " << id << " 出现了 " << count[id] << " 次，应为 100";
    }

    bag_destroy(&bag);
}

// P4-23 7-bag 的核心保证：任意连续 **13** 个方块里，七种编号都至少出现一次
// 这就是规格里承诺的"最多等 12 块一定能等到你想要的形状"。
//
// 【为什么是 13 而不是 12 —— 这里的推导值得记下来】
// 一袋是 7 个位置。某一段连续取块里"每种都出现过"，等价于这段里**完整包含至少一袋**。
// 设窗口从第 s 个位置开始、长度为 L，则窗口是 [s, s+L-1]，
// 它完整包含某一袋 [7k, 7k+6] 的条件是：7k 落在区间 [s, s+L-7] 里。
// 一个长度为 7 的整数区间必然包含 7 的倍数，所以要求 (L-7)+1 ≥ 7，即 L ≥ 13。
//
// 换句话说：最坏的间隔出现在"某块是这袋的第一个、又是下一袋的最后一个"——
// 位置相差 13，中间隔着 12 块。所以窗口取 13 才是正确的；
// 取 12 会漏掉这种最坏情况（本用例最初就写成 12，被自己抓了出来）。
TEST(Phase4Bag, P4_23_Every12ConsecutiveContainAllSeven)
{
    Bag bag;
    bag_init(&bag, 1002003);

    const std::vector<int> pieces = TakeMany(&bag, 700);
    ASSERT_EQ((int)pieces.size(), 700);

    const int window = 13;
    for (int start = 0; start + window <= (int)pieces.size(); start++)
    {
        bool seen[TETROMINO_COUNT] = { false };
        for (int k = 0; k < window; k++)
        {
            seen[pieces[start + k]] = true;
        }

        for (int id = 0; id < TETROMINO_COUNT; id++)
        {
            ASSERT_TRUE(seen[id])
                << "从第 " << start << " 个位置起的 13 个方块里没有出现编号 " << id;
        }
    }

    bag_destroy(&bag);
}

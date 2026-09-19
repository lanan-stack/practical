// ============================================================
// bag.c —— 7-bag 袋子系统（实现）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib）
//
// 核心流程：
//   bag_init  → 洗一袋（7 个编号）压入循环队列
//   bag_next  → 出队一个（这就是"下一个出场的方块"）
//               出队后若队列剩余 ≤ 3，再洗一袋压入队尾
//   bag_peek  → 查看队头第 0/1/2 个，用于画面上的预览
// ============================================================
#include "bag.h"
#include <assert.h>
#include <stddef.h>

// ============================================================
// 随机数：xorshift32
// ------------------------------------------------------------
// 为什么自己写而不是用 rand()：rand() 的种子无法可靠地"注入"，
// 同一串调用在不同运行环境下不保证一致，测试就没法复现。
// xorshift32 只有几行，状态就是种子，同一状态必然产生同一串数字。
// ============================================================
unsigned int bag_rng_next(unsigned int* state)
{
    assert(state);

    unsigned int x = *state;
    if (x == 0)
    {
        // 保护：xorshift 的状态为 0 时会永远输出 0，随机数就废了。
        // 这里换成一个固定的非零常数，保证种子 0 也能正常工作。
        x = 0x9E3779B9u;
    }

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    *state = x;
    return x;
}

// ============================================================
// 洗牌：Fisher–Yates
// ------------------------------------------------------------
// 做法：从最后一个位置开始，往前逐个处理；每个位置与"它自己或它前面"的
//       某个随机位置交换。这样每种排列出现的概率都相同（无偏）。
//
// 注意 r % (i+1) 这一取模：因为要交换的下标必须落在 0~i 之内，
//       所以用 (i+1) 取模，绝不能写成 % i（那样最后一位永远换不到）。
// ============================================================
void bag_shuffle(int ids[], int count, unsigned int* state)
{
    assert(ids);
    assert(state);
    if (count <= 1)
    {
        return;   // 0 个或 1 个元素没什么可打乱的
    }

    for (int i = count - 1; i > 0; i--)
    {
        unsigned int r = bag_rng_next(state);
        int j = (int)(r % (unsigned int)(i + 1));   // j 落在 0~i

        int tmp = ids[i];
        ids[i] = ids[j];
        ids[j] = tmp;
    }
}

// ============================================================
// 内部函数：洗出一整袋压入队尾
// ------------------------------------------------------------
// 一袋 = 七种编号各一个，顺序随机。
// 压入队尾（而不是插到前面），所以已经在排队的方块顺序不会被搅乱 ——
// 这一点很关键：它保证了"预览窗口只会整体前移"，不会因为补袋而跳变。
// ============================================================
static void bag_push_one_new_bag(Bag* bag)
{
    int ids[TETROMINO_COUNT];
    for (int i = 0; i < TETROMINO_COUNT; i++)
    {
        ids[i] = i;   // 0,1,2,3,4,5,6
    }

    bag_shuffle(ids, TETROMINO_COUNT, &bag->rngState);

    for (int i = 0; i < TETROMINO_COUNT; i++)
    {
        // 队列容量足够（最多 3 + 7 = 10 ≤ 14），这里不会失败
        circular_queue_push(bag->queue, ids[i]);
    }
}

// ============================================================
// 初始化 / 销毁
// ============================================================
void bag_init(Bag* bag, unsigned int seed)
{
    assert(bag);

    // 建立承载方块的循环队列（容量两袋）
    bag->queue = circular_queue_create(BAG_QUEUE_CAPACITY);

    // 记录种子；种子 0 换成固定常数，避免退化成"永远输出 0"
    bag->rngState = (seed == 0) ? 0x9E3779B9u : seed;

    // 开局的队列里先放一袋（7 个）
    bag_push_one_new_bag(bag);
}

void bag_destroy(Bag* bag)
{
    assert(bag);
    if (bag->queue != NULL)
    {
        circular_queue_destroy(bag->queue);
        bag->queue = NULL;
    }
}

// ============================================================
// 取下一块
// ------------------------------------------------------------
// 顺序很重要：先出队，再判断要不要补袋。
//   · 出队得到的就是"下一个出场的方块"；
//   · 出队后剩余 ≤ 3 就补一整袋，保证预览永远够用。
// 最坏情况的元素个数：3（补袋前）+ 7 = 10，容量 14 绰绰有余。
// ============================================================
bool bag_next(Bag* bag, int* outId)
{
    assert(bag);
    if (outId == NULL)
    {
        return false;
    }

    if (!circular_queue_pop_value(bag->queue, outId))
    {
        return false;   // 正常不会发生：袋子永远不会空
    }

    if (circular_queue_size(bag->queue) <= BAG_REFILL_THRESHOLD)
    {
        bag_push_one_new_bag(bag);
    }
    return true;
}

// ============================================================
// 预览第 index 个方块
// ------------------------------------------------------------
// 只允许看队头往后 0~2 这三个 —— 与画面上的预览数量一致。
// 越界一律返回 false，不写 outId。
// ============================================================
bool bag_peek(const Bag* bag, int index, int* outId)
{
    assert(bag);
    if (outId == NULL)
    {
        return false;
    }
    if (index < 0 || index >= BAG_PREVIEW_COUNT)
    {
        return false;
    }
    return circular_queue_peek_at(bag->queue, index, outId);
}

// ============================================================
// 查询队列里当前待用的方块个数
// ============================================================
int bag_queue_size(const Bag* bag)
{
    assert(bag);
    return circular_queue_size(bag->queue);
}

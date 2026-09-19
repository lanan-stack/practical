// ============================================================
// bag.h —— 7-bag 袋子系统（逻辑层）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib，可独立单元测试）
//
// 职责：决定"下一个出场的是哪一块"。
// 做法（这是现代俄罗斯方块的通行做法，俗称 7-bag）：
//   · 把七种方块编号 0~6 洗牌，得到一"袋"（袋内每种恰好一个）；
//   · 按袋依次出块，因此"最多等 12 块一定能等到你想要的形状"；
//   · 用**循环队列**承载待用的方块：出块 = 出队，补袋 = 整袋入队，
//     预览 = 查看队头第 0/1/2 个。
//
// 边界：本模块只管"方块从哪来"，不管棋盘、碰撞、消行，也不管暂存槽（那是 hold 模块）。
// ============================================================
#ifndef BAG_H
#define BAG_H

#include <stdbool.h>
#include "circularqueue.h"   // 复用项目原有的循环队列
#include "tetromino.h"       // 方块编号与 TETROMINO_COUNT（其中已包含 config.h 的棋盘常量）

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// 参数（数值依据 project.md 第 5.3 节）
// ------------------------------------------------------------
// BAG_QUEUE_CAPACITY：队列容量取"两袋"= 14。
//    为什么是两袋：取块后判断"剩余 ≤ 3 就补一整袋（7 个）"，
//    最坏情况 3 + 7 = 10 个，14 有富余，永远不会溢出。
// BAG_REFILL_THRESHOLD：剩余 ≤ 3 时补袋。3 正好等于预览数量，
//    保证任何时刻都够显示 3 个预览。
// ============================================================
#define BAG_QUEUE_CAPACITY   14   // 队列容量（两袋）
#define BAG_REFILL_THRESHOLD 3    // 剩余元素 <= 此值时补一袋
#define BAG_PREVIEW_COUNT    3    // 预览数量

// 袋子对象
//   queue    —— 承载待用方块编号的循环队列
//   rngState —— 随机数状态（由种子决定，同种子必然得到同一串方块，便于测试复现）
typedef struct Bag
{
    circular_queue* queue;
    unsigned int    rngState;
} Bag;

// ---------- 随机数（xorshift32，可注入状态，便于复现）----------
// 参数：state —— 随机数状态指针（会被就地更新）
// 返回：本次产生的随机数
// 说明：内部对"状态为 0"做了保护 —— xorshift 类算法在状态为 0 时会永远输出 0，
//       那样洗牌就彻底失效了，所以状态为 0 时先换成一个固定常数。
unsigned int bag_rng_next(unsigned int* state);

// ---------- 洗牌（Fisher–Yates，原地打乱）----------
// 参数：ids   —— 待打乱的数组
//       count —— 元素个数
//       state —— 随机数状态
// 说明：从最后一个元素往前，每个位置与它之前（含自身）的随机一个位置交换，
//       这样每种排列出现的概率相同。
void bag_shuffle(int ids[], int count, unsigned int* state);

// ---------- 初始化 / 销毁 ----------
// 参数：seed —— 随机数种子。同一个种子必然得到同一串方块（可复现）。
void bag_init(Bag* bag, unsigned int seed);
void bag_destroy(Bag* bag);

// ---------- 取下一块 ----------
// 参数：outId —— 输出：方块编号 0~6
// 返回：true = 成功；false = 失败（队列异常为空或 outId 为 NULL）
// 说明：取出后如果队列剩余不足，会自动补一整袋（补在队尾，不影响队头顺序）。
bool bag_next(Bag* bag, int* outId);

// ---------- 预览第 index 个方块 ----------
// 参数：index —— 0 表示"下一个要出场的"，依次往后；合法范围 0~2
//       outId —— 输出：方块编号
// 返回：true = 成功；false = index 越界（负数或 ≥ 预览数量）
bool bag_peek(const Bag* bag, int index, int* outId);

// ---------- 查询队列当前的待用元素个数（测试与调试用）----------
int bag_queue_size(const Bag* bag);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // BAG_H

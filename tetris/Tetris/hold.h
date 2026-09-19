// ============================================================
// hold.h —— 暂存（Hold）规则（逻辑层）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib，可独立单元测试）
//
// 三条规则（依据 project.md 第 2.4 节）：
//   1. 暂存槽为空时按下暂存：当前方块存入槽里，然后**预览的第 0 个方块立刻出场**；
//   2. 暂存槽非空时按下暂存：当前方块与槽里那块**互换**；
//   3. **每个方块只能换一次**：换过之后必须等该方块落地锁定，才能再换
//      （否则可以狂按 G，方块永远不下落，一局就卡死了）。
//
// 边界：本模块只管编号的交换与"能不能换"；
//       至于"换出来的方块旋转姿态要归零"，属于游戏状态（Phase 6）的职责。
// ============================================================
#ifndef HOLD_H
#define HOLD_H

#include <stdbool.h>
#include "bag.h"

#ifdef __cplusplus
extern "C" {
#endif

// 暂存状态
//   slotId    —— 槽里的方块编号（hasPiece 为 false 时无意义）
//   hasPiece  —— 槽里有没有方块
//   canSwap   —— 当前方块还能不能换（每个方块只能换一次）
typedef struct Hold
{
    int  slotId;
    bool hasPiece;
    bool canSwap;
} Hold;

// ---------- 初始化 ----------
// 槽为空、允许换（开局第一块就可以暂存）。
void hold_init(Hold* hold);

// ---------- 按下暂存键 ----------
// 参数：hold      —— 暂存状态
//       bag       —— 袋子（槽为空时要从这里取下一块）
//       currentId —— 输入输出参数：进函数时是"当前方块编号"，
//                    成功时会被改成"换进来的方块编号"
// 返回：true = 换成功（currentId 已更新）；false = 本次不允许换（currentId 不变）
//
// 失败的情形：本方块已经换过一次，或袋子取块失败。
// 失败时保证"什么都不改"（槽内容、canSwap、currentId 全部保持原样）。
bool hold_apply(Hold* hold, Bag* bag, int* currentId);

// ---------- 方块落地锁定时调用 ----------
// 作用：解锁"每个方块只能换一次"的限制，让下一块又能暂存。
void hold_mark_locked(Hold* hold);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // HOLD_H

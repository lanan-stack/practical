// ============================================================
// hold.c —— 暂存（Hold）规则的实现
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib）
// ============================================================
#include "hold.h"
#include <assert.h>
#include <stddef.h>

// ============================================================
// 初始化
// ============================================================
void hold_init(Hold* hold)
{
    assert(hold);
    hold->slotId = 0;
    hold->hasPiece = false;
    hold->canSwap = true;   // 开局第一块就能暂存
}

// ============================================================
// 按下暂存键
// ------------------------------------------------------------
// 两种情形：
//   槽为空：先把"下一块"从袋子里取出来（必须先取，取不到就整体放弃，
//           这样才保证失败时不留下半成品状态），
//           再把当前块放进槽，最后把新取到的块作为当前块。
//   槽非空：当前块与槽里的块互换，袋子不动（所以这时候队列元素数不变）。
// 无论哪种情形，最后都把 canSwap 置为 false —— 这就是"每个方块只能换一次"。
// ============================================================
bool hold_apply(Hold* hold, Bag* bag, int* currentId)
{
    assert(hold);
    assert(bag);
    if (currentId == NULL)
    {
        return false;
    }

    // 规则 3：本方块已经换过一次，本次直接拒绝，什么都不改
    if (!hold->canSwap)
    {
        return false;
    }

    if (!hold->hasPiece)
    {
        // ---------- 规则 1：槽为空 ----------
        int nextId = 0;
        if (!bag_next(bag, &nextId))
        {
            return false;   // 取不到方块：整体放弃，保持原状
        }

        hold->slotId = *currentId;   // 当前块进槽
        hold->hasPiece = true;
        *currentId = nextId;         // 新取到的块成为当前块
    }
    else
    {
        // ---------- 规则 2：槽非空，两块互换 ----------
        int tmp = hold->slotId;
        hold->slotId = *currentId;
        *currentId = tmp;
    }

    hold->canSwap = false;
    return true;
}

// ============================================================
// 方块落地锁定时解锁
// ============================================================
void hold_mark_locked(Hold* hold)
{
    assert(hold);
    hold->canSwap = true;
}

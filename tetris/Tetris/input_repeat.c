// ============================================================
// input_repeat.c —— 按键长按连发的实现
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib）
// ============================================================
#include "input_repeat.h"
#include "config.h"

// ============================================================
// 初始化
// ============================================================
void input_repeat_init(InputRepeat* repeat)
{
    repeat->wasHeld = 0;
    repeat->heldSeconds = 0.0;
    repeat->repeating = 0;
    repeat->repeatTimer = 0.0;
}

// ============================================================
// 每帧更新
// ------------------------------------------------------------
// 分三种情况：
//   1) 未按住：清零所有计时，返回 0。
//   2) 刚按下（这一帧按下而上一帧没按）：立刻返回 1（走一格），并开始计时。
//   3) 一直按住：累计时间；
//        · 还没到 DAS：不动；
//        · 刚到 DAS：连发一次，并进入连发阶段；
//        · 已进入连发：每累计满 ARR 就走一格（用 while 是因为
//          万一某一帧特别长（掉帧），可能要补走好几格）。
//
// 注意"松开"的处理：一旦松手，heldSeconds 与 repeating 全部清零，
// 于是下次按下又重新走一遍"立刻一格 + 等 DAS"的流程（用例 P5-27）。
// ============================================================
int input_repeat_update(InputRepeat* repeat, int held, double dt)
{
    // ---------- 情况 1：没按住 ----------
    if (!held)
    {
        repeat->wasHeld = 0;
        repeat->heldSeconds = 0.0;
        repeat->repeating = 0;
        repeat->repeatTimer = 0.0;
        return 0;
    }

    // ---------- 情况 2：刚按下 ----------
    if (!repeat->wasHeld)
    {
        repeat->wasHeld = 1;
        repeat->heldSeconds = 0.0;
        repeat->repeating = 0;
        repeat->repeatTimer = 0.0;
        return 1;   // 按下瞬间先走一格
    }

    // ---------- 情况 3：持续按住 ----------
    repeat->heldSeconds += dt;

    if (!repeat->repeating)
    {
        if (repeat->heldSeconds < INPUT_DAS_SECONDS)
        {
            return 0;   // 还在等待期内
        }

        // 到点了：进入连发阶段，并立刻连发一次
        repeat->repeating = 1;
        repeat->repeatTimer = 0.0;
        return 1;
    }

    // 连发阶段：每累计满 ARR 走一格（一帧掉太长时可能补走多格）
    repeat->repeatTimer += dt;

    int moves = 0;
    while (repeat->repeatTimer >= INPUT_ARR_SECONDS)
    {
        repeat->repeatTimer -= INPUT_ARR_SECONDS;
        moves++;
    }
    return moves;
}

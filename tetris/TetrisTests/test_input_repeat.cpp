// ============================================================
// test_input_repeat.cpp —— Phase 5A 单元测试：按键长按连发（DAS / ARR）
// ------------------------------------------------------------
// 对应用例 P5-25 ~ P5-27（清单已由用户审核通过）。
//
// 被测代码：input_repeat.h / input_repeat.c
//
// 说明：下面的用例**全部从 config.h 的常量推导时间**，不写死具体秒数。
//       这样以后调手感参数（比如把 DAS 从 0.20 改成 0.15）时，
//       测试会自动跟着走，不会因为"写死的期望值"而误报失败。
// ============================================================

#include <gtest/gtest.h>

#include "input_repeat.h"
#include "config.h"

// ============================================================
// P5-25 首次延迟：按下瞬间走一格，之后 0.17 秒内不再重复
// ============================================================
TEST(Phase5Input, P5_25_InitialDelay)
{
    InputRepeat r;
    input_repeat_init(&r);

    const double frame = 0.01;

    // 按下瞬间：应当立刻走一格
    EXPECT_EQ(input_repeat_update(&r, 1, frame), 1) << "按下瞬间应移动一格";

    // 继续按住：在 DAS 之前都不该再动。
    // 这里留出 0.02 秒的余量，避免浮点累加恰好压在临界点上。
    int movesBeforeDas = 0;
    double elapsed = 0.0;
    while (elapsed + frame <= INPUT_DAS_SECONDS - 0.02)
    {
        movesBeforeDas += input_repeat_update(&r, 1, frame);
        elapsed += frame;
    }
    EXPECT_EQ(movesBeforeDas, 0) << "DAS 时间之内不该重复移动";
}

// ============================================================
// P5-26 连发间隔：超过 0.17 秒后，每 0.08 秒移动一格
// ============================================================
TEST(Phase5Input, P5_26_RepeatInterval)
{
    InputRepeat r;
    input_repeat_init(&r);

    const double frame = 0.01;
    EXPECT_EQ(input_repeat_update(&r, 1, frame), 1);   // 按下瞬间

    // 一直推进到刚好跨过 DAS：期间应当恰好连发一次
    int moves = 0;
    double elapsed = 0.0;
    while (elapsed < INPUT_DAS_SECONDS)
    {
        moves += input_repeat_update(&r, 1, frame);
        elapsed += frame;
    }
    EXPECT_EQ(moves, 1) << "跨过 DAS 时应当连发一次";

    // 此后每 ARR 秒一格
    moves = 0;
    elapsed = 0.0;
    while (elapsed < INPUT_ARR_SECONDS)
    {
        moves += input_repeat_update(&r, 1, frame);
        elapsed += frame;
    }
    EXPECT_EQ(moves, 1) << "接下来的一个 ARR 周期应当再移动一格";

    // 再来两个 ARR 周期，应当正好两格
    moves = 0;
    elapsed = 0.0;
    while (elapsed < INPUT_ARR_SECONDS * 2.0)
    {
        moves += input_repeat_update(&r, 1, frame);
        elapsed += frame;
    }
    EXPECT_EQ(moves, 2) << "两个 ARR 周期应当移动两格";
}

// ============================================================
// P5-27 松开重按：计时从头开始，不会立刻连发
// ============================================================
TEST(Phase5Input, P5_27_ReleaseRestartsTiming)
{
    InputRepeat r;
    input_repeat_init(&r);

    const double frame = 0.01;

    // 先按住足够久，进入连发状态
    EXPECT_EQ(input_repeat_update(&r, 1, frame), 1);
    double elapsed = 0.0;
    while (elapsed < INPUT_DAS_SECONDS + INPUT_ARR_SECONDS)
    {
        input_repeat_update(&r, 1, frame);
        elapsed += frame;
    }

    // 松开一帧
    EXPECT_EQ(input_repeat_update(&r, 0, frame), 0);
    EXPECT_EQ(r.repeating, 0) << "松开后连发状态应清零";

    // 重新按下：应当只走一格（按下瞬间），随后重新进入 DAS 等待期
    EXPECT_EQ(input_repeat_update(&r, 1, frame), 1) << "重按瞬间走一格";

    int moves = 0;
    elapsed = 0.0;
    while (elapsed + frame <= INPUT_DAS_SECONDS - 0.02)   // 仍在 DAS 之内
    {
        moves += input_repeat_update(&r, 1, frame);
        elapsed += frame;
    }
    EXPECT_EQ(moves, 0) << "重按之后必须重新等待 DAS，不能立刻连发";
}

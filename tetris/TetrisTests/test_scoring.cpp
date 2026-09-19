// ============================================================
// test_scoring.cpp —— Phase 6 单元测试：难度、速度表、等级、计分、连击
// ------------------------------------------------------------
// 对应用例 P6-01 ~ P6-09（A / B 组）、P6-19 ~ P6-27（D 组）、
//          P6-29 ~ P6-32（F 组），共 22 条。
// 清单已由用户两轮审核通过。
// ============================================================

#include <gtest/gtest.h>

#include "game_logic.h"
#include "test_board_helpers.h"

// 走完一次"消掉 lineCount 行"的完整流程（含闪烁）
static void DoClear(GameState* g, int lineCount)
{
    SetupClear(g, lineCount);
    LockNow(g);
    FinishClearing(g);
}

// ============================================================
// A 组：难度与速度表
// ============================================================

// P6-01 三档的封顶等级
TEST(Phase6Difficulty, P6_01_CapLevels)
{
    EXPECT_EQ(difficulty_cap_level(DIFFICULTY_EASY), 5);
    EXPECT_EQ(difficulty_cap_level(DIFFICULTY_NORMAL), 8);
    EXPECT_EQ(difficulty_cap_level(DIFFICULTY_HARD), 11);
}

// P6-02 速度表 1~11 级
TEST(Phase6Difficulty, P6_02_SpeedTable)
{
    const double expected[11] =
    {
        1.00, 0.79, 0.62, 0.47, 0.36, 0.26, 0.19, 0.13, 0.094, 0.064, 0.043
    };

    GameState g;
    game_init(&g, 12345);
    game_set_difficulty(&g, DIFFICULTY_HARD);   // 挑战档封顶在 11 级，整张表都能查到

    for (int level = 1; level <= 11; level++)
    {
        g.level = level;   // 白盒：直接把等级设成要查的档位
        EXPECT_NEAR(game_gravity_seconds(&g), expected[level - 1], 1e-9)
            << "第 " << level << " 级的速度不对";
    }

    game_destroy(&g);
}

// P6-03 轻松档封顶
TEST(Phase6Difficulty, P6_03_EasyIsCapped)
{
    GameState g;
    game_init(&g, 12345);
    game_set_difficulty(&g, DIFFICULTY_EASY);

    g.level = 5;
    const double atCap = game_gravity_seconds(&g);
    g.level = 20;
    EXPECT_NEAR(game_gravity_seconds(&g), atCap, 1e-9) << "轻松档 5 级之后不该再变快";
    EXPECT_NEAR(atCap, 0.36, 1e-9);

    game_destroy(&g);
}

// P6-04 标准档封顶
TEST(Phase6Difficulty, P6_04_NormalIsCapped)
{
    GameState g;
    game_init(&g, 12345);
    game_set_difficulty(&g, DIFFICULTY_NORMAL);

    g.level = 8;
    const double atCap = game_gravity_seconds(&g);
    g.level = 20;
    EXPECT_NEAR(game_gravity_seconds(&g), atCap, 1e-9) << "标准档 8 级之后不该再变快";
    EXPECT_NEAR(atCap, 0.13, 1e-9);

    game_destroy(&g);
}

// P6-05 挑战档封顶
TEST(Phase6Difficulty, P6_05_HardIsCapped)
{
    GameState g;
    game_init(&g, 12345);
    game_set_difficulty(&g, DIFFICULTY_HARD);

    g.level = 11;
    const double atCap = game_gravity_seconds(&g);
    g.level = 20;
    EXPECT_NEAR(game_gravity_seconds(&g), atCap, 1e-9) << "挑战档 11 级之后不该再变快";
    EXPECT_NEAR(atCap, 0.043, 1e-9);

    game_destroy(&g);
}

// P6-06 等级再高也不会比封顶值更快（秒数不会小于封顶值）
TEST(Phase6Difficulty, P6_06_NeverFasterThanCap)
{
    const Difficulty all[DIFFICULTY_COUNT] =
    {
        DIFFICULTY_EASY, DIFFICULTY_NORMAL, DIFFICULTY_HARD
    };

    for (Difficulty d : all)
    {
        GameState g;
        game_init(&g, 12345);
        game_set_difficulty(&g, d);

        g.level = difficulty_cap_level(d);
        const double capSeconds = game_gravity_seconds(&g);

        for (int level = 1; level <= 40; level++)
        {
            g.level = level;
            EXPECT_GE(game_gravity_seconds(&g), capSeconds - 1e-9)
                << "难度 " << (int)d << " 在等级 " << level << " 时比封顶值还快";
        }

        game_destroy(&g);
    }
}

// ============================================================
// B 组：等级与行数
// ============================================================

// P6-07 开局状态
TEST(Phase6Level, P6_07_InitialState)
{
    GameState g;
    game_init(&g, 12345);

    EXPECT_EQ(g.level, 1);
    EXPECT_EQ(g.totalLines, 0);
    EXPECT_EQ(g.score, 0);
    EXPECT_EQ(g.comboCount, 0);

    game_destroy(&g);
}

// P6-08 每消 10 行升一级
TEST(Phase6Level, P6_08_LevelUpEveryTenLines)
{
    GameState g;
    game_init(&g, 12345);

    for (int i = 0; i < 10; i++)
    {
        DoClear(&g, 1);
    }
    EXPECT_EQ(g.totalLines, 10);
    EXPECT_EQ(g.level, 2) << "消满 10 行应当升到 2 级";

    for (int i = 0; i < 9; i++)
    {
        DoClear(&g, 1);
    }
    EXPECT_EQ(g.totalLines, 19);
    EXPECT_EQ(g.level, 2) << "19 行仍是 2 级";

    DoClear(&g, 1);
    EXPECT_EQ(g.totalLines, 20);
    EXPECT_EQ(g.level, 3) << "20 行应当升到 3 级";

    game_destroy(&g);
}

// P6-09 行数累计
TEST(Phase6Level, P6_09_LineCountAccumulates)
{
    GameState g;
    game_init(&g, 12345);

    DoClear(&g, 1);
    EXPECT_EQ(g.totalLines, 1);

    DoClear(&g, 2);
    EXPECT_EQ(g.totalLines, 3);

    game_destroy(&g);
}

// ============================================================
// D 组：计分与连击
// ============================================================

// P6-19 消 1 行 = 100 × 等级
TEST(Phase6Score, P6_19_OneLine)
{
    GameState g;
    game_init(&g, 12345);
    DoClear(&g, 1);
    EXPECT_EQ(g.score, 100);
    game_destroy(&g);
}

// P6-20 消 2 行 = 300 × 等级
TEST(Phase6Score, P6_20_TwoLines)
{
    GameState g;
    game_init(&g, 12345);
    DoClear(&g, 2);
    EXPECT_EQ(g.score, 300);
    game_destroy(&g);
}

// P6-21 消 3 行 = 500 × 等级
TEST(Phase6Score, P6_21_ThreeLines)
{
    GameState g;
    game_init(&g, 12345);
    DoClear(&g, 3);
    EXPECT_EQ(g.score, 500);
    game_destroy(&g);
}

// P6-22 消 4 行 = 800 × 等级
TEST(Phase6Score, P6_22_FourLines)
{
    GameState g;
    game_init(&g, 12345);
    DoClear(&g, 4);
    EXPECT_EQ(g.score, 800);
    game_destroy(&g);
}

// P6-23 计分用"消行前"的等级（基础分与连击都算在内）
TEST(Phase6Score, P6_23_UsesLevelBeforeClearing)
{
    // ---------- 情形一：基础分 ----------
    {
        GameState g;
        game_init(&g, 12345);

        for (int i = 0; i < 8; i++)
        {
            DoClear(&g, 1);          // 累计 8 行，仍是 1 级
        }
        ASSERT_EQ(g.level, 1);
        LockWithoutClear(&g);        // 把连击清零，单独看基础分

        const int before = g.score;
        DoClear(&g, 4);              // 8 + 4 = 12 行 → 升到 2 级

        EXPECT_EQ(g.totalLines, 12);
        EXPECT_EQ(g.level, 2);
        EXPECT_EQ(g.score - before, 800) << "应当按消行前的 1 级算，而不是 2 级";

        game_destroy(&g);
    }

    // ---------- 情形二：连击加成也用消行前的等级 ----------
    {
        GameState g;
        game_init(&g, 12345);

        for (int i = 0; i < 8; i++)
        {
            DoClear(&g, 1);          // 8 次连续消行 → 连击 8
        }
        ASSERT_EQ(g.comboCount, 8);
        ASSERT_EQ(g.level, 1);

        const int before = g.score;
        DoClear(&g, 4);              // 连击变 9，行数变 12 → 升到 2 级

        // 基础分 800×1 + 连击 50×(9-1)×1 = 1200
        EXPECT_EQ(g.score - before, 1200)
            << "连击加成也应当按消行前的 1 级算（若按 2 级会得到 2400）";

        game_destroy(&g);
    }
}

// P6-24 分数累加（不因升级而重算）
TEST(Phase6Score, P6_24_ScoreAccumulates)
{
    GameState g;
    game_init(&g, 12345);

    DoClear(&g, 1);   // 100（连击 1，无加成）
    EXPECT_EQ(g.score, 100);

    DoClear(&g, 1);   // 100 + 连击加成 50×1×1 = 150
    EXPECT_EQ(g.score, 250);

    game_destroy(&g);
}

// P6-25 连击加成：连续第二次消行额外 +50×(n-1)×等级
TEST(Phase6Score, P6_25_ComboBonus)
{
    GameState g;
    game_init(&g, 12345);

    DoClear(&g, 1);
    ASSERT_EQ(g.comboCount, 1);

    const int before = g.score;
    DoClear(&g, 1);

    EXPECT_EQ(g.comboCount, 2);
    EXPECT_EQ(g.score - before, 150) << "第二次应当是 100 + 50×(2-1)×1";

    game_destroy(&g);
}

// P6-26 连击中断：中间一次没消行就归零
TEST(Phase6Score, P6_26_ComboResets)
{
    GameState g;
    game_init(&g, 12345);

    DoClear(&g, 1);
    ASSERT_EQ(g.comboCount, 1);
    ASSERT_EQ(g.score, 100);

    LockWithoutClear(&g);
    EXPECT_EQ(g.comboCount, 0) << "没消行应当把连击清零";
    EXPECT_EQ(g.score, 100) << "不消行不加分";

    DoClear(&g, 1);
    EXPECT_EQ(g.comboCount, 1);
    EXPECT_EQ(g.score, 200) << "连击已归零，这次不该有连击加成";

    game_destroy(&g);
}

// P6-27 一次消 4 行只算 1 次连击
TEST(Phase6Score, P6_27_FourLinesCountAsOneCombo)
{
    GameState g;
    game_init(&g, 12345);

    DoClear(&g, 4);
    EXPECT_EQ(g.comboCount, 1) << "一次消 4 行仍然只算 1 次连击";
    ASSERT_EQ(g.score, 800);

    DoClear(&g, 4);
    EXPECT_EQ(g.comboCount, 2);
    // 第二次：800 + 50×(2-1)×1 = 850
    EXPECT_EQ(g.score, 1650) << "若把 4 行算成 4 次连击，加成就变成 350 了";

    game_destroy(&g);
}

// ============================================================
// F 组：难度、等级与速度的联动
// ============================================================

// P6-29 难度不影响得分
TEST(Phase6Difficulty, P6_29_DifficultyDoesNotAffectScore)
{
    const Difficulty all[DIFFICULTY_COUNT] =
    {
        DIFFICULTY_EASY, DIFFICULTY_NORMAL, DIFFICULTY_HARD
    };

    for (Difficulty d : all)
    {
        GameState g;
        game_init(&g, 12345);
        game_set_difficulty(&g, d);
        DoClear(&g, 1);
        EXPECT_EQ(g.score, 100) << "难度 " << (int)d << " 的得分应与难度无关";
        game_destroy(&g);
    }
}

// P6-30 切换难度立即生效
TEST(Phase6Difficulty, P6_30_SwitchingDifficultyTakesEffectImmediately)
{
    GameState g;
    game_init(&g, 12345);
    g.level = 9;   // 白盒：直接设到 9 级（超过轻松档的封顶 5 级）

    game_set_difficulty(&g, DIFFICULTY_EASY);
    EXPECT_NEAR(game_gravity_seconds(&g), 0.36, 1e-9) << "轻松档 9 级应按 5 级封顶算";

    game_set_difficulty(&g, DIFFICULTY_HARD);
    EXPECT_NEAR(game_gravity_seconds(&g), 0.094, 1e-9) << "换成挑战档后应立刻按 9 级算";

    game_destroy(&g);
}

// P6-31 升级后下落速度立即生效
TEST(Phase6Level, P6_31_SpeedUpdatesAfterLevelUp)
{
    GameState g;
    game_init(&g, 12345);
    EXPECT_NEAR(game_gravity_seconds(&g), 1.00, 1e-9) << "1 级应当是 1.00 秒/格";

    for (int i = 0; i < 10; i++)
    {
        DoClear(&g, 1);   // 消到 10 行 → 升到 2 级
    }
    ASSERT_EQ(g.level, 2);

    EXPECT_NEAR(game_gravity_seconds(&g), 0.79, 1e-9)
        << "升到 2 级后下落间隔应当立刻变成 0.79 秒";

    game_destroy(&g);
}

// P6-32 消行出新块后，下落计时从零开始
TEST(Phase6Clearing, P6_32_TimerResetsAfterClearing)
{
    GameState g;
    game_init(&g, 12345);
    DoClear(&g, 1);   // 消 1 行（等级仍是 1，间隔 1.00 秒）

    ASSERT_EQ(g.level, 1);
    const int rowAfterClear = g.pieceRow;

    // 新方块刚出现，推进"不足一个下落间隔"的时间就不该下落
    // （间隔由当前等级推导，不写死秒数）
    game_update(&g, game_gravity_seconds(&g) * 0.9, 0);
    EXPECT_EQ(g.pieceRow, rowAfterClear)
        << "新方块应当从零开始计时；若它下落了，说明上一块的计时被带了过来";

    game_destroy(&g);
}

// ============================================================
// test_clearing.cpp —— Phase 6 单元测试：消行规则与消行与顶出的先后
// ------------------------------------------------------------
// 对应用例 P6-10 ~ P6-18（C 组）与 P6-28（E 组），共 10 条。
// 清单已由用户两轮审核通过。
//
// 被测代码：game_logic.c 里的消行与闪烁阶段
// ============================================================

#include <gtest/gtest.h>

#include "game_logic.h"
#include "test_board_helpers.h"

// ============================================================
// P6-10 消 1 行：该行消失，上方方块整体下移一行
// ============================================================
TEST(Phase6Clearing, P6_10_SingleLineClearShiftsRowsDown)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 1);
    g.board[LAST_ROW - 1][0] = TETROMINO_S;   // 在底行之上一格放个标记块

    LockNow(&g);
    ASSERT_TRUE(game_is_clearing(&g)) << "消行应当先进入闪烁阶段";
    EXPECT_EQ(g.totalLines, 1);

    FinishClearing(&g);
    EXPECT_FALSE(game_is_clearing(&g));
    EXPECT_EQ(g.board[LAST_ROW][0], TETROMINO_S) << "上方的标记块应整体下移一行";
    EXPECT_EQ(g.board[LAST_ROW - 1][0], BOARD_EMPTY) << "原来的位置应该空出来";

    game_destroy(&g);
}

// ============================================================
// P6-11 一次消 2 行
// ============================================================
TEST(Phase6Clearing, P6_11_TwoLinesAtOnce)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 2);
    g.board[LAST_ROW - 2][0] = TETROMINO_S;   // 更上方一个标记块

    LockNow(&g);
    EXPECT_EQ(g.totalLines, 2);
    FinishClearing(&g);

    EXPECT_EQ(g.board[LAST_ROW][0], TETROMINO_S) << "两行消失后，上方方块应下移两行";
    EXPECT_EQ(g.board[LAST_ROW - 1][0], BOARD_EMPTY);

    game_destroy(&g);
}

// ============================================================
// P6-12 一次消 3 行
// ============================================================
TEST(Phase6Clearing, P6_12_ThreeLinesAtOnce)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 3);
    g.board[LAST_ROW - 3][0] = TETROMINO_S;

    LockNow(&g);
    EXPECT_EQ(g.totalLines, 3);
    FinishClearing(&g);

    EXPECT_EQ(g.board[LAST_ROW][0], TETROMINO_S) << "三行消失后，上方方块应下移三行";

    game_destroy(&g);
}

// ============================================================
// P6-13 一次消 4 行
// ============================================================
TEST(Phase6Clearing, P6_13_FourLinesAtOnce)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 4);
    g.board[LAST_ROW - 4][0] = TETROMINO_S;

    LockNow(&g);
    EXPECT_EQ(g.totalLines, 4);
    FinishClearing(&g);

    EXPECT_EQ(g.board[LAST_ROW][0], TETROMINO_S) << "四行消失后，上方方块应下移四行";

    game_destroy(&g);
}

// ============================================================
// P6-14 只消满行：未填满的行原样保留
// ============================================================
TEST(Phase6Clearing, P6_14_OnlyFullRowsAreCleared)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 2);

    // 倒数第三行故意留一个洞
    FillRowKeeping(&g, LAST_ROW - 2, { 9 });
    ASSERT_EQ(g.board[LAST_ROW - 2][9], BOARD_EMPTY);

    LockNow(&g);
    EXPECT_EQ(g.totalLines, 2) << "只有下面两行是满的";
    FinishClearing(&g);

    // 那行带洞的行被整体下移两行，但洞还在 —— 说明它没有被误消
    EXPECT_EQ(g.board[LAST_ROW][9], BOARD_EMPTY) << "未填满的行不该被消掉";
    EXPECT_NE(g.board[LAST_ROW][0], BOARD_EMPTY);

    game_destroy(&g);
}

// ============================================================
// P6-15 多行下移后，上下顺序不变
// ============================================================
TEST(Phase6Clearing, P6_15_OrderIsPreserved)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 2);

    // 在被消的行**上方**，沿第 0 列竖着放三个不同颜色的标记块（从下到上：Z、S、T）
    g.board[LAST_ROW - 2][0] = TETROMINO_Z;
    g.board[LAST_ROW - 3][0] = TETROMINO_S;
    g.board[LAST_ROW - 4][0] = TETROMINO_T;

    LockNow(&g);
    FinishClearing(&g);

    // 三个标记块整体下移两行，相对顺序保持不变（T 仍在上、Z 仍在下）
    EXPECT_EQ(g.board[LAST_ROW][0], TETROMINO_Z);
    EXPECT_EQ(g.board[LAST_ROW - 1][0], TETROMINO_S);
    EXPECT_EQ(g.board[LAST_ROW - 2][0], TETROMINO_T);

    game_destroy(&g);
}

// ============================================================
// P6-16 不连续的多行同时被消除（最容易写错的情形）
// ============================================================
TEST(Phase6Clearing, P6_16_NonAdjacentRowsClearedTogether)
{
    GameState g;
    game_init(&g, 12345);
    ClearBoard(&g);

    // 倒数第 4 行满、倒数第 3 行有个洞、倒数第 2 行满
    FillRowKeeping(&g, LAST_ROW - 3, { });        // 满行
    FillRowKeeping(&g, LAST_ROW - 2, { 3 });      // 有洞
    FillRowKeeping(&g, LAST_ROW - 1, { });        // 满行

    // 让一个方块落在最上面（不制造新的满行）
    SetPieceAt(&g, TETROMINO_O, 3, 0);
    game_hard_drop(&g);
    FinishClearing(&g);

    EXPECT_EQ(g.totalLines, 2) << "应消掉不连续的两行";

    // 带洞的那行原本在倒数第 3 行，消掉下方一行后它落到倒数第 2 行，洞仍然在
    EXPECT_EQ(g.board[LAST_ROW - 1][3], BOARD_EMPTY) << "带洞的行应当保留并整体下移";
    EXPECT_NE(g.board[LAST_ROW - 1][0], BOARD_EMPTY) << "该行其余部分应当仍然填着";
    EXPECT_EQ(g.board[LAST_ROW][0], BOARD_EMPTY) << "最底行原本是空的，下移后仍是空的";

    game_destroy(&g);
}

// ============================================================
// P6-17 闪烁阶段：棋盘内容不变、行号标记正确、不产生新方块
// ============================================================
TEST(Phase6Clearing, P6_17_FlashPhaseKeepsBoardIntact)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 1);
    const int pieceBefore = g.pieceId;

    LockNow(&g);
    ASSERT_TRUE(game_is_clearing(&g));

    // 满行还在棋盘上（没有被提前移除）
    for (int c = 0; c < BOARD_COLS; c++)
    {
        EXPECT_NE(g.board[LAST_ROW][c], BOARD_EMPTY) << "闪烁期间该行不该消失";
    }

    // 行号标记正确：只有最后一行被标记
    EXPECT_TRUE(game_row_is_clearing(&g, LAST_ROW));
    EXPECT_FALSE(game_row_is_clearing(&g, LAST_ROW - 1));

    // 闪烁期间推进时间：不产生新方块（pieceId 不变）
    game_update(&g, CLEAR_FLASH_SECONDS * 0.5, 0);
    EXPECT_TRUE(game_is_clearing(&g));
    EXPECT_EQ(g.pieceId, pieceBefore) << "闪烁期间不该换新方块";
    EXPECT_TRUE(game_clear_progress(&g) > 0.0);
    EXPECT_TRUE(game_clear_progress(&g) < 1.0);

    game_destroy(&g);
}

// ============================================================
// P6-18 闪烁结束：行才真正移除，并立即出新块
// ============================================================
TEST(Phase6Clearing, P6_18_ClearingFinishesAndSpawnsNext)
{
    GameState g;
    game_init(&g, 12345);
    SetupClear(&g, 1);
    const int lockedId = g.pieceId;

    LockNow(&g);
    ASSERT_TRUE(game_is_clearing(&g));

    // 差一点点：还不该结束
    game_update(&g, CLEAR_FLASH_SECONDS - 0.001, 0);
    EXPECT_TRUE(game_is_clearing(&g)) << "时间没到不该结束闪烁";

    // 到点：行被移除，并出了新方块（位置在出生点）
    game_update(&g, 0.002, 0);
    EXPECT_FALSE(game_is_clearing(&g));
    // 被消掉的是最底行；它上方的那些格子（含刚锁定的方块下半部分）整体落到最底行
    EXPECT_EQ(g.board[LAST_ROW][0], BOARD_EMPTY) << "原本填满的最底行应当已经消失";
    EXPECT_EQ(g.board[LAST_ROW][4], lockedId) << "上方的方块应当落到底行";
    EXPECT_NE(g.pieceId, lockedId) << "消行结束后应当出了新方块";
    EXPECT_EQ(g.pieceCol, SPAWN_BOX_COL);
    EXPECT_EQ(g.pieceRow, SPAWN_BOX_ROW);

    game_destroy(&g);
}

// ============================================================
// P6-28 先消行、再判顶出：靠消行腾出空间时不算顶出
// ------------------------------------------------------------
// 局面：出生区里的行 2、3、4 都被填满（其中行 4 还差最后四格），
//       行 5 的前四格垫着，让这一块 I 形落不下去、直接锁定。
// 这一手落下后行 2~4 全部凑满 → 消掉 → 出生区被腾空 → 不该判顶出。
// 如果实现成"先判顶出、后消行"，这里就会被误判为游戏结束。
// ============================================================
TEST(Phase6Clearing, P6_28_ClearHappensBeforeTopOutCheck)
{
    GameState g;
    game_init(&g, 12345);
    ClearBoard(&g);

    // 出生区（逻辑行 1~4）里：行 1~3 满，行 4 只差第 0~3 列
    FillRowKeeping(&g, 1, { });
    FillRowKeeping(&g, 2, { });
    FillRowKeeping(&g, 3, { });
    FillRowKeeping(&g, 4, { 0, 1, 2, 3 });

    // 行 5 的前四格垫着，让 I 形停在第 4 行
    for (int c = 0; c < 4; c++)
    {
        g.board[5][c] = TETROMINO_J;
    }

    // 横放的 I 形：占盒子第 1 行 → 放在逻辑行 3 时正好落在第 4 行
    SetPieceAt(&g, TETROMINO_I, 0, 3);
    ASSERT_FALSE(game_can_place(&g, g.pieceBox, g.pieceCol, g.pieceRow + 1))
        << "它应该正好卡住、落不下去";

    LockNow(&g);
    ASSERT_TRUE(game_is_clearing(&g));
    EXPECT_EQ(g.totalLines, 4) << "行 1~4 都满了";

    FinishClearing(&g);

    EXPECT_FALSE(g.gameOver) << "消行腾出了空间，不该判顶出";
    EXPECT_EQ(g.pieceCol, SPAWN_BOX_COL) << "新方块应当正常出现在出生点";
    EXPECT_EQ(g.pieceRow, SPAWN_BOX_ROW);

    game_destroy(&g);
}

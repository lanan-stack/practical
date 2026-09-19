// ============================================================
// test_game_logic.cpp —— Phase 5A 单元测试：棋盘、碰撞、移动、下落、锁定
// ------------------------------------------------------------
// 对应用例 P5-01 ~ P5-24、P5-28（清单已由用户两轮审核通过）。
// 连发相关的 P5-25 ~ P5-27 在 test_input_repeat.cpp 里。
//
// 被测代码：game_logic.h / game_logic.c
//
// 【关于测试里直接改状态】
// 有些用例需要构造特定局面（比如"棋盘第 21 行第 4 列已经有一块"），
// 所以下面的辅助函数会直接写棋盘、直接搬动方块位置。
// 这属于白盒测试的常规做法：先把局面摆好，再验证被测函数的行为。
// 注意摆局面时不要调用被测函数本身，否则就成了"用自己验证自己"。
// ============================================================

#include <gtest/gtest.h>

#include "game_logic.h"
#include "config.h"
#include "tetromino.h"

// ============================================================
// 测试辅助
// ============================================================

// 把当前方块换成指定形状（姿态为初始）
static void SetPiece(GameState* g, int id)
{
    g->pieceId = id;
    tetromino_base_copy(id, g->pieceBox);
}

// 把当前方块搬到指定位置（盒子左上角）
static void PlacePiece(GameState* g, int col, int row)
{
    g->pieceCol = col;
    g->pieceRow = row;
}

// 当前方块在棋盘上占用的最小/最大列、最大行
static int MinCellCol(const GameState* g)
{
    int v = BOARD_COLS;
    for (int r = 0; r < TETROMINO_BOX; r++)
        for (int c = 0; c < TETROMINO_BOX; c++)
            if (g->pieceBox[r][c] && g->pieceCol + c < v)
                v = g->pieceCol + c;
    return v;
}

static int MaxCellCol(const GameState* g)
{
    int v = -1;
    for (int r = 0; r < TETROMINO_BOX; r++)
        for (int c = 0; c < TETROMINO_BOX; c++)
            if (g->pieceBox[r][c] && g->pieceCol + c > v)
                v = g->pieceCol + c;
    return v;
}

// 棋盘上某编号占了多少格
static int CountId(const GameState* g, int id)
{
    int n = 0;
    for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
        for (int c = 0; c < BOARD_COLS; c++)
            if (g->board[r][c] == id)
                n++;
    return n;
}

// 棋盘上非空格的总数
static int CountFilled(const GameState* g)
{
    int n = 0;
    for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
        for (int c = 0; c < BOARD_COLS; c++)
            if (g->board[r][c] != BOARD_EMPTY)
                n++;
    return n;
}

// 把某个形状摆到"方块的底边刚好贴住棋盘底"的位置，用于落地相关用例
static void PlaceOnFloor(GameState* g)
{
    int lowest = 0;
    for (int r = 0; r < TETROMINO_BOX; r++)
        for (int c = 0; c < TETROMINO_BOX; c++)
            if (g->pieceBox[r][c] && r > lowest)
                lowest = r;
    PlacePiece(g, 3, BOARD_TOTAL_ROWS - 1 - lowest);
}

// ============================================================
// A 组：碰撞检测
// ============================================================

// P5-01 空棋盘：出生位置可放置，且不会越界
TEST(Phase5Collision, P5_01_SpawnPositionIsPlaceable)
{
    GameState g;
    game_init(&g, 12345);

    EXPECT_EQ(g.pieceCol, SPAWN_BOX_COL);
    EXPECT_EQ(g.pieceRow, SPAWN_BOX_ROW);
    EXPECT_TRUE(game_can_place(&g, g.pieceBox, g.pieceCol, g.pieceRow));
    EXPECT_GE(MinCellCol(&g), 0);
    EXPECT_LT(MaxCellCol(&g), BOARD_COLS);

    game_destroy(&g);
}

// P5-02 左右边界：贴墙之后不能再移
TEST(Phase5Collision, P5_02_HorizontalBounds)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_I);
    PlacePiece(&g, 0, 50 % BOARD_TOTAL_ROWS);   // 放在棋盘中间，远离底部

    // 一路向左，直到走不动
    while (game_move(&g, -1)) { }
    EXPECT_FALSE(game_move(&g, -1)) << "贴左墙后不应再能左移";
    EXPECT_EQ(MinCellCol(&g), 0) << "最左格应正好贴住第 0 列";

    // 一路向右，直到走不动
    while (game_move(&g, +1)) { }
    EXPECT_FALSE(game_move(&g, +1)) << "贴右墙后不应再能右移";
    EXPECT_EQ(MaxCellCol(&g), BOARD_COLS - 1) << "最右格应正好贴住最后一列";

    game_destroy(&g);
}

// P5-03 底部边界：再往下一格就越出逻辑棋盘
TEST(Phase5Collision, P5_03_BottomBoundary)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlaceOnFloor(&g);   // O 的底边贴在最后一行

    EXPECT_TRUE(game_can_place(&g, g.pieceBox, g.pieceCol, g.pieceRow));
    EXPECT_FALSE(game_can_place(&g, g.pieceBox, g.pieceCol, g.pieceRow + 1))
        << "再下移一格就该越出棋盘底部";

    game_destroy(&g);
}

// P5-04 与已锁定方块重叠
TEST(Phase5Collision, P5_04_BlockedByLockedCells)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);

    // 在 (行 10, 列 5) 放一块已锁定的方块
    g.board[10][5] = TETROMINO_J;

    // O 占盒子第 1~2 行、第 1~2 列；把它放到会让右下角压住 (10,5) 的位置
    EXPECT_FALSE(game_can_place(&g, g.pieceBox, 3, 8)) << "应检测到与已锁定方块重叠";

    // 往上挪一格就不冲突了
    EXPECT_TRUE(game_can_place(&g, g.pieceBox, 3, 7));

    game_destroy(&g);
}

// P5-05 隐藏缓冲行允许站立；逻辑行小于 0 视为越界（防御性检查）
TEST(Phase5Collision, P5_05_BufferRowsAreUsable)
{
    GameState g;
    game_init(&g, 12345);

    // I 形占盒子第 1 行，盒子放在逻辑行 -1 时，占用的格子在逻辑行 0（缓冲行）→ 合法
    SetPiece(&g, TETROMINO_I);
    EXPECT_TRUE(game_can_place(&g, g.pieceBox, 3, -1))
        << "缓冲行（逻辑第 0 行）应当允许放置";

    // T 形占盒子第 0~1 行，盒子放在逻辑行 -1 时，会有一格落到逻辑行 -1 → 越界
    SetPiece(&g, TETROMINO_T);
    EXPECT_FALSE(game_can_place(&g, g.pieceBox, 3, -1))
        << "逻辑行小于 0 必须判为越界";

    game_destroy(&g);
}

// ============================================================
// B 组：移动与旋转
// ============================================================

// P5-06 左右移动成功
TEST(Phase5Move, P5_06_MoveSucceeds)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_T);
    PlacePiece(&g, 3, 5);

    const int rowBefore = g.pieceRow;
    EXPECT_TRUE(game_move(&g, +1));
    EXPECT_EQ(g.pieceCol, 4);
    EXPECT_EQ(g.pieceRow, rowBefore) << "左右移动不该改变行号";
    EXPECT_TRUE(game_move(&g, -1));
    EXPECT_EQ(g.pieceCol, 3);

    game_destroy(&g);
}

// P5-07 移动被挡：状态一点都不变
TEST(Phase5Move, P5_07_BlockedMoveChangesNothing)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlacePiece(&g, -1, 5);   // O 占列 0~1，已经贴左墙

    int boxBefore[TETROMINO_BOX][TETROMINO_BOX];
    for (int r = 0; r < TETROMINO_BOX; r++)
        for (int c = 0; c < TETROMINO_BOX; c++)
            boxBefore[r][c] = g.pieceBox[r][c];

    EXPECT_FALSE(game_move(&g, -1));
    EXPECT_EQ(g.pieceCol, -1);
    EXPECT_EQ(g.pieceRow, 5);
    for (int r = 0; r < TETROMINO_BOX; r++)
        for (int c = 0; c < TETROMINO_BOX; c++)
            EXPECT_EQ(g.pieceBox[r][c], boxBefore[r][c]) << "被挡住的移动不该改形状";

    game_destroy(&g);
}

// P5-08 旋转成功，且方向与 tetromino 模块一致（用 I 形精确坐标验证）
TEST(Phase5Move, P5_08_RotationDirectionIsCorrect)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_I);
    PlacePiece(&g, 3, 2);

    // 顺时针：I 形应由横躺变竖立，且竖条落在盒子的第 2 列
    ASSERT_TRUE(game_rotate(&g, true));
    EXPECT_EQ(g.pieceBox[0][2], 1);
    EXPECT_EQ(g.pieceBox[1][2], 1);
    EXPECT_EQ(g.pieceBox[2][2], 1);
    EXPECT_EQ(g.pieceBox[3][2], 1);
    EXPECT_EQ(g.pieceBox[1][0], 0) << "横躺那一行应该清空了";
    EXPECT_EQ(g.pieceCol, 3) << "旋转不该改变位置";

    // 逆时针转回来：应回到横躺
    ASSERT_TRUE(game_rotate(&g, false));
    EXPECT_EQ(g.pieceBox[1][0], 1);
    EXPECT_EQ(g.pieceBox[1][1], 1);
    EXPECT_EQ(g.pieceBox[1][2], 1);
    EXPECT_EQ(g.pieceBox[1][3], 1);

    game_destroy(&g);
}

// P5-09 旋转被挡：两种真实情形（贴右墙 / 贴地），形状与位置都不变
TEST(Phase5Move, P5_09_BlockedRotationChangesNothing)
{
    // ---------- 情形一：贴右墙 ----------
    {
        GameState g;
        game_init(&g, 12345);
        SetPiece(&g, TETROMINO_T);            // T 占盒子第 0~2 列
        PlacePiece(&g, BOARD_COLS - 3, 5);    // 占满最右三列

        int colBefore = g.pieceCol;
        int rowBefore = g.pieceRow;
        EXPECT_FALSE(game_rotate(&g, true))
            << "T 顺时针会用到盒子第 3 列，贴右墙时应该转不动";
        EXPECT_EQ(g.pieceCol, colBefore);
        EXPECT_EQ(g.pieceRow, rowBefore);
        EXPECT_EQ(g.pieceBox[0][1], 1) << "转不动时形状必须保持原样";

        game_destroy(&g);
    }

    // ---------- 情形二：贴地面 ----------
    {
        GameState g;
        game_init(&g, 12345);
        SetPiece(&g, TETROMINO_I);
        PlaceOnFloor(&g);   // 横躺，占逻辑第 20 行

        EXPECT_FALSE(game_rotate(&g, true))
            << "I 形顺时针会变成竖立（占 4 行），贴地时应该转不动";
        EXPECT_EQ(g.pieceBox[1][0], 1) << "转不动时形状必须保持原样";
        EXPECT_EQ(g.pieceBox[0][2], 0);

        game_destroy(&g);
    }
}

// ============================================================
// C 组：下落与锁定
// ============================================================

// P5-10 重力：按当前等级的速度下落，并且**余量会被保留**
// 说明：时间全部由 game_gravity_seconds() 推导，不写死秒数 ——
//       以后调速度表（比如把 1 级改快一点）时，测试会自动跟着走。
TEST(Phase5Fall, P5_10_GravityAndRemainder)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlacePiece(&g, 3, 0);

    const double interval = game_gravity_seconds(&g);

    game_update(&g, interval * 0.9, 0);
    EXPECT_EQ(g.pieceRow, 0) << "不足一个间隔不该下落";

    game_update(&g, interval * 0.3, 0);   // 累计 1.2 个间隔 → 落一格，余下 0.2
    EXPECT_EQ(g.pieceRow, 1);

    // 关键：余量必须保留。再推进 0.9 个间隔（累计 1.1）就该再落一格；
    // 如果余量被丢掉，这一步不会动。
    const int rowBefore = g.pieceRow;
    game_update(&g, interval * 0.9, 0);
    EXPECT_EQ(g.pieceRow, rowBefore + 1) << "上一格剩下的时间没有被保留";

    game_destroy(&g);
}

// P5-11 软降：下落速度变成正常的 SOFT_DROP_MULTIPLIER 倍
// 说明：这里的时间全部由常量推导，不写死秒数 —— 以后调软降倍率时，
//       测试会自动跟着走，不会因为"写死的期望值"而误报失败。
TEST(Phase5Fall, P5_11_SoftDropIsTenTimesFaster)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlacePiece(&g, 3, 0);

    const double baseInterval = game_gravity_seconds(&g);          // 1 级：1.00 秒/格
    const double softInterval = baseInterval / SOFT_DROP_MULTIPLIER; // 软降：约 0.14 秒/格

    // 按住软降：一个软降间隔之后应落一格
    game_update(&g, softInterval + 0.001, 1);
    EXPECT_EQ(g.pieceRow, 1) << "按住软降时一个软降间隔应落一格";

    // 同样的时长，不按软降则不该动
    g.fallTimer = 0.0;
    game_update(&g, softInterval + 0.001, 0);
    EXPECT_EQ(g.pieceRow, 1) << "同样的时间，不按软降不该下落";

    // 按住软降两个间隔：应当正好落两格
    g.fallTimer = 0.0;
    game_update(&g, softInterval * 2.0 + 0.001, 1);
    EXPECT_EQ(g.pieceRow, 3) << "两个软降间隔应落两格";

    game_destroy(&g);
}

// P5-12 落地判定：贴底算落地；**只有部分格子下方有障碍**也算落地
TEST(Phase5Fall, P5_12_GroundDetection)
{
    // ---------- 贴底 ----------
    {
        GameState g;
        game_init(&g, 12345);
        SetPiece(&g, TETROMINO_O);
        PlaceOnFloor(&g);
        game_update(&g, 0.0, 0);
        EXPECT_TRUE(g.onGround);
        game_destroy(&g);
    }

    // ---------- 只有部分格子下方有障碍（用不对称的 L 形）----------
    {
        GameState g;
        game_init(&g, 12345);
        SetPiece(&g, TETROMINO_L);
        // L 形占盒子第 0 行的第 2 列、第 1 行的第 0~2 列
        PlacePiece(&g, 3, 10);
        // L 在棋盘上的格子：行 10 列 5；行 11 列 3、4、5
        // 只在行 12 列 4 放一块障碍 —— 其余下方都是空的
        g.board[12][4] = TETROMINO_Z;

        EXPECT_FALSE(game_can_place(&g, g.pieceBox, g.pieceCol, g.pieceRow + 1))
            << "只要有任意一格下方被挡住，就不能整体下移";

        game_update(&g, 0.0, 0);
        EXPECT_TRUE(g.onGround) << "部分受阻也应判为在地面上";
        game_destroy(&g);
    }
}

// P5-13 锁定延迟：延迟时间内不锁定也不下移，到点才锁定（时间由常量推导）
TEST(Phase5Fall, P5_13_LockDelay)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_L);
    PlaceOnFloor(&g);

    const int rowBefore = g.pieceRow;
    const int id = g.pieceId;
    const double delay = LOCK_DELAY_SECONDS;

    game_update(&g, delay * 0.8, 0);
    EXPECT_EQ(g.pieceRow, rowBefore) << "在地面上不该继续下落";
    EXPECT_EQ(CountId(&g, id), 0) << "还没到延迟时间不该锁定";

    game_update(&g, delay * 0.3, 0);   // 累计 1.1 倍延迟
    EXPECT_EQ(CountId(&g, id), 4) << "延迟到点应锁定并写入棋盘";

    game_destroy(&g);
}

// P5-14 锁定延迟重置上限：成功操作可重置，最多 8 次
TEST(Phase5Fall, P5_14_LockResetIsCapped)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_L);
    PlaceOnFloor(&g);
    const int id = g.pieceId;

    const double wait = LOCK_DELAY_SECONDS * 0.8;

    // 连续 8 次"等一会儿 + 成功移动"：每次都会把锁定计时清零
    for (int i = 0; i < LOCK_RESET_MAX; i++)
    {
        game_update(&g, wait, 0);
        ASSERT_TRUE(game_move(&g, (i % 2 == 0) ? +1 : -1)) << "第 " << i << " 次移动应当成功";
    }
    EXPECT_EQ(g.lockResetCount, LOCK_RESET_MAX) << "重置次数应为 8";
    EXPECT_EQ(CountId(&g, id), 0) << "还没锁定";

    // 第 9 次：计时不再被清零
    game_update(&g, wait, 0);
    ASSERT_TRUE(game_move(&g, +1));
    EXPECT_GT(g.lockTimer, wait * 0.5) << "重置次数用尽后，锁定计时不该再被清零";

    // 再推进一个完整的延迟时长：必定锁定
    game_update(&g, LOCK_DELAY_SECONDS, 0);
    EXPECT_EQ(CountId(&g, id), 4) << "重置次数用尽后应按原计时锁定";

    game_destroy(&g);
}

// P5-15 被挡住的移动/旋转不重置锁定计时
TEST(Phase5Fall, P5_15_BlockedActionDoesNotResetLockTimer)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlacePiece(&g, -1, 5);   // 贴左墙，且不在地面上

    // 先让它落到地面（在下方铺一块，形成"地面"）
    for (int c = 0; c < BOARD_COLS; c++)
        g.board[BOARD_TOTAL_ROWS - 1][c] = TETROMINO_J;
    PlaceOnFloor(&g);
    game_update(&g, 0.0, 0);
    ASSERT_TRUE(g.onGround);

    game_update(&g, LOCK_DELAY_SECONDS * 0.8, 0);
    const double timerBefore = g.lockTimer;
    const int resetBefore = g.lockResetCount;

    EXPECT_FALSE(game_move(&g, -1)) << "贴左墙时左移应当失败";
    EXPECT_FALSE(game_move(&g, -1));

    EXPECT_DOUBLE_EQ(g.lockTimer, timerBefore) << "失败的移动不该重置锁定计时";
    EXPECT_EQ(g.lockResetCount, resetBefore) << "失败的移动不该计入重置次数";

    game_destroy(&g);
}

// P5-16 落地后移出悬空 → 恢复下落状态、锁定计时清零
TEST(Phase5Fall, P5_16_LeavingGroundClearsLockTimer)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);

    // 造一根两格高的柱子：行 20、21 的列 4
    g.board[20][4] = TETROMINO_Z;
    g.board[21][4] = TETROMINO_Z;

    // O 摆成"坐在柱子上"：占了行 19~20、列 4~5 → 右下角正压着柱子
    PlacePiece(&g, 3, 18);
    game_update(&g, 0.0, 0);
    ASSERT_TRUE(g.onGround) << "应当坐在柱子上";

    game_update(&g, LOCK_DELAY_SECONDS * 0.6, 0);
    ASSERT_GT(g.lockTimer, 0.0);

    // 向右挪一格：不再压着柱子 → 下面就空了
    ASSERT_TRUE(game_move(&g, +1));
    EXPECT_FALSE(g.onGround) << "挪开之后应该是悬空的";
    EXPECT_DOUBLE_EQ(g.lockTimer, 0.0) << "悬空后锁定计时必须清零";
    EXPECT_EQ(g.lockResetCount, 0);

    game_destroy(&g);
}

// P5-17 硬降：立即落到最低合法位置并立即锁定
TEST(Phase5Fall, P5_17_HardDropLocksImmediately)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlacePiece(&g, 3, 2);
    const int id = g.pieceId;

    game_hard_drop(&g);

    // O 占盒子第 1~2 行 → 落到逻辑行 19 时，底边正好是第 21 行
    EXPECT_EQ(CountId(&g, id), 4) << "硬降应当立即锁定并写入棋盘";
    EXPECT_EQ(g.board[20][4], id);
    EXPECT_EQ(g.board[20][5], id);
    EXPECT_EQ(g.board[21][4], id);
    EXPECT_EQ(g.board[21][5], id);

    game_destroy(&g);
}

// P5-18 锁定写入的是方块编号；随后立即出新块，新块在出生位置
TEST(Phase5Fall, P5_18_LockWritesIdsAndSpawnsNext)
{
    GameState g;
    game_init(&g, 12345);

    // 第一块：用 L 形硬降
    SetPiece(&g, TETROMINO_L);
    const int firstId = g.pieceId;
    game_hard_drop(&g);
    EXPECT_EQ(CountId(&g, firstId), 4) << "棋盘里应记录该方块的编号（不是简单的 1）";

    // 第二块：换个编号，再硬降
    SetPiece(&g, TETROMINO_S);
    const int secondId = g.pieceId;
    ASSERT_NE(firstId, secondId);
    game_hard_drop(&g);

    EXPECT_EQ(CountId(&g, firstId), 4);
    EXPECT_EQ(CountId(&g, secondId), 4);
    EXPECT_EQ(CountFilled(&g), 8) << "两块方块应各自留下 4 格";

    // 出新块：位置回到出生点
    EXPECT_EQ(g.pieceCol, SPAWN_BOX_COL);
    EXPECT_EQ(g.pieceRow, SPAWN_BOX_ROW);
    EXPECT_FALSE(g.gameOver);

    game_destroy(&g);
}

// ============================================================
// D 组：幽灵落点
// ============================================================

// P5-19 空棋盘的幽灵：底边贴合棋盘底
TEST(Phase5Ghost, P5_19_GhostOnEmptyBoard)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlacePiece(&g, 3, 2);

    // O 占盒子第 1~2 行，底边落在第 21 行 → 盒子左上角在逻辑行 19
    EXPECT_EQ(game_ghost_row(&g), 19);
    EXPECT_EQ(g.pieceRow, 2) << "算幽灵不该真的把方块搬走";

    game_destroy(&g);
}

// P5-20 有堆垛时的幽灵：落在堆垛正上方，而且"再下一格就放不下"
TEST(Phase5Ghost, P5_20_GhostStopsOnStack)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);
    PlacePiece(&g, 3, 2);

    // 底行第 4 列放一块障碍
    g.board[BOARD_TOTAL_ROWS - 1][4] = TETROMINO_Z;

    const int ghost = game_ghost_row(&g);
    EXPECT_EQ(ghost, 18) << "底边应停在障碍上方";
    EXPECT_TRUE(game_can_place(&g, g.pieceBox, g.pieceCol, ghost));
    EXPECT_FALSE(game_can_place(&g, g.pieceBox, g.pieceCol, ghost + 1))
        << "幽灵位置再往下一格必须放不下";

    game_destroy(&g);
}

// ============================================================
// E 组：暂存联动
// ============================================================

// P5-21 空槽暂存：新当前方块 = 原预览第 0 个，且在出生位置
TEST(Phase5Hold, P5_21_HoldTakesFromBag)
{
    GameState g;
    game_init(&g, 12345);

    int expectedNext = -1;
    ASSERT_TRUE(bag_peek(&g.bag, 0, &expectedNext));
    const int currentBefore = g.pieceId;

    ASSERT_TRUE(game_hold_piece(&g));

    EXPECT_TRUE(g.hold.hasPiece);
    EXPECT_EQ(g.hold.slotId, currentBefore);
    EXPECT_EQ(g.pieceId, expectedNext);
    EXPECT_EQ(g.pieceCol, SPAWN_BOX_COL);
    EXPECT_EQ(g.pieceRow, SPAWN_BOX_ROW);

    game_destroy(&g);
}

// P5-22 非空槽暂存：换进来的方块回到出生位置且**姿态归零**
TEST(Phase5Hold, P5_22_SwappedInPieceResetsRotation)
{
    GameState g;
    game_init(&g, 12345);
    const int pieceA = g.pieceId;

    // 第一次暂存：A 进槽，取到 B
    ASSERT_TRUE(game_hold_piece(&g));
    const int pieceB = g.pieceId;

    // 把 B 转一下（此时场上姿态不再是初始姿态）
    ASSERT_TRUE(game_rotate(&g, true));
    ASSERT_FALSE(g.pieceBox[1][0] == 1 && g.pieceBox[1][1] == 1 && g.pieceBox[1][2] == 1);

    // 模拟 B 落地锁定 → 可以再换
    hold_mark_locked(&g.hold);

    // 第二次暂存：A 换回来
    ASSERT_TRUE(game_hold_piece(&g));
    EXPECT_EQ(g.pieceId, pieceA);
    EXPECT_EQ(g.hold.slotId, pieceB);

    // 关键：换进来的 A 必须是初始姿态、且在出生位置
    int base[TETROMINO_BOX][TETROMINO_BOX];
    ASSERT_TRUE(tetromino_base_copy(pieceA, base));
    for (int r = 0; r < TETROMINO_BOX; r++)
        for (int c = 0; c < TETROMINO_BOX; c++)
            EXPECT_EQ(g.pieceBox[r][c], base[r][c]) << "换进来的方块必须是初始姿态";

    EXPECT_EQ(g.pieceCol, SPAWN_BOX_COL);
    EXPECT_EQ(g.pieceRow, SPAWN_BOX_ROW);

    game_destroy(&g);
}

// P5-23 暂存后锁定计时清零、重新处于空中
TEST(Phase5Hold, P5_23_HoldClearsTimers)
{
    GameState g;
    game_init(&g, 12345);

    // 让当前方块落地并累计一点锁定计时
    SetPiece(&g, TETROMINO_O);
    PlaceOnFloor(&g);
    game_update(&g, LOCK_DELAY_SECONDS * 0.6, 0);
    ASSERT_TRUE(g.onGround);
    ASSERT_GT(g.lockTimer, 0.0);

    ASSERT_TRUE(game_hold_piece(&g));

    EXPECT_DOUBLE_EQ(g.lockTimer, 0.0) << "暂存后锁定计时必须清零";
    EXPECT_DOUBLE_EQ(g.fallTimer, 0.0);
    EXPECT_FALSE(g.onGround) << "换进来的方块在出生位置，应当是空中的";

    game_destroy(&g);
}

// P5-24 暂存后本块不能再换，锁定后恢复
TEST(Phase5Hold, P5_24_HoldOncePerPiece)
{
    GameState g;
    game_init(&g, 12345);

    ASSERT_TRUE(game_hold_piece(&g));
    EXPECT_FALSE(game_hold_piece(&g)) << "同一块不允许换第二次";

    hold_mark_locked(&g.hold);   // 模拟该方块落地
    EXPECT_TRUE(game_hold_piece(&g)) << "锁定之后应恢复可换";

    game_destroy(&g);
}

// ============================================================
// G 组：游戏结束
// ============================================================

// P5-28 顶出：出生位置被占 → 结束，且此后不再产生新方块
TEST(Phase5Over, P5_28_TopOutEndsGame)
{
    GameState g;
    game_init(&g, 12345);
    SetPiece(&g, TETROMINO_O);

    // 把当前方块挪到下方（避开出生区域）
    PlacePiece(&g, 3, 18);

    // 把出生区域（逻辑行 1~4、列 3~6）塞满
    for (int r = 1; r <= 4; r++)
        for (int c = SPAWN_BOX_COL; c < SPAWN_BOX_COL + TETROMINO_BOX; c++)
            g.board[r][c] = TETROMINO_J;

    game_hard_drop(&g);   // 锁定当前块 → 出新块时出生位置被占

    EXPECT_TRUE(g.gameOver) << "出生位置被占应当判为顶出";

    // 结束后再推进时间：棋盘与当前方块都不应再变化
    const int filledBefore = CountFilled(&g);
    const int colBefore = g.pieceCol;
    const int rowBefore = g.pieceRow;
    game_update(&g, 5.0, 0);
    EXPECT_EQ(CountFilled(&g), filledBefore) << "结束后不该再锁定新方块";
    EXPECT_EQ(g.pieceCol, colBefore);
    EXPECT_EQ(g.pieceRow, rowBefore);

    game_destroy(&g);
}

// ============================================================
// test_board_helpers.h —— Phase 6 两组用例共用的"摆局面"辅助函数
// ------------------------------------------------------------
// 为什么需要它们：要验证"消 1/2/3/4 行"，就得先造出"某一手落下后正好凑成几行满"
// 的局面。与其在每个用例里重复十几行摆棋盘的代码，不如集中在这里，
// 让用例本体只剩下"断言"。
//
// 这些都是**白盒测试**手法：直接改棋盘、直接搬方块。
// 注意：摆局面时不要调用被测函数本身，否则就成了"用自己验证自己"。
// ============================================================
#ifndef TEST_BOARD_HELPERS_H
#define TEST_BOARD_HELPERS_H

#include <initializer_list>

#include "game_logic.h"

// 棋盘最后一行的逻辑行号
#define LAST_ROW  (BOARD_TOTAL_ROWS - 1)

// ---------- 清空整张棋盘（不改变分数、等级等状态）----------
static inline void ClearBoard(GameState* g)
{
    for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
    {
        for (int c = 0; c < BOARD_COLS; c++)
        {
            g->board[r][c] = BOARD_EMPTY;
        }
    }
}

// ---------- 填满某一行，但保留 keepEmpty 里的列不填 ----------
// 这是构造"差几格就满"局面的核心工具：留下的空位正好由待测方块补上。
static inline void FillRowKeeping(GameState* g, int row,
                                  std::initializer_list<int> keepEmpty,
                                  int id = TETROMINO_J)
{
    for (int c = 0; c < BOARD_COLS; c++)
    {
        g->board[row][c] = id;
    }
    for (int c : keepEmpty)
    {
        g->board[row][c] = BOARD_EMPTY;
    }
}

// ---------- 把当前方块换成指定形状并摆到指定位置 ----------
static inline void SetPieceAt(GameState* g, int id, int col, int row)
{
    g->pieceId = id;
    tetromino_base_copy(id, g->pieceBox);
    g->pieceCol = col;
    g->pieceRow = row;
}

// ---------- 让当前方块落地并走完锁定延迟 ----------
// 返回后：若这一手消了行，游戏处于"消行闪烁阶段"；否则已经出了下一块。
static inline void LockNow(GameState* g)
{
    game_update(g, 0.0, 0);                        // 刷新落地状态
    game_update(g, LOCK_DELAY_SECONDS + 0.001, 0); // 走完锁定延迟 → 锁定
}

// ---------- 走完消行闪烁 ----------
static inline void FinishClearing(GameState* g)
{
    game_update(g, CLEAR_FLASH_SECONDS + 0.001, 0);
}

// ---------- 摆好"这一手落下后正好消掉 lineCount 行"的局面 ----------
// lineCount 取 1~4。摆完之后当前方块就位，但还没锁定。
//
// 各档的构造原理：
//   1 行：底行留出 4、5 两列，用 O 形补齐
//   2 行：最后两行都留出 4、5 两列，用 O 形补齐
//   3 行：最后三行分别留出缺口，用顺时针转过的 L 形补齐
//   4 行：最后四行都留出第 0 列，用竖起来的 I 形补齐
static inline void SetupClear(GameState* g, int lineCount)
{
    ClearBoard(g);

    switch (lineCount)
    {
        case 1:
            FillRowKeeping(g, LAST_ROW, { 4, 5 });
            SetPieceAt(g, TETROMINO_O, 3, LAST_ROW - 2);
            break;

        case 2:
            FillRowKeeping(g, LAST_ROW - 1, { 4, 5 });
            FillRowKeeping(g, LAST_ROW, { 4, 5 });
            SetPieceAt(g, TETROMINO_O, 3, LAST_ROW - 2);
            break;

        case 3:
            FillRowKeeping(g, LAST_ROW - 2, { 5 });
            FillRowKeeping(g, LAST_ROW - 1, { 5 });
            FillRowKeeping(g, LAST_ROW, { 5, 6 });
            SetPieceAt(g, TETROMINO_L, 3, LAST_ROW - 2);
            game_rotate(g, true);   // L 顺时针后占 (19,5)(20,5)(21,5)(21,6)
            break;

        case 4:
            for (int r = LAST_ROW - 3; r <= LAST_ROW; r++)
            {
                FillRowKeeping(g, r, { 0 });
            }
            // 注意顺序：如果先把 I 摆到下面那几行再旋转，它会和已经填好的格子重叠、
            // 旋转判定失败，结果整块摆错位置。所以先放在棋盘上方的空处旋转，
            // 再把它搬到目标列与目标行。
            SetPieceAt(g, TETROMINO_I, 3, 0);
            game_rotate(g, true);       // I 顺时针 → 竖立（占盒子第 2 列）
            g->pieceCol = -2;           // 竖条落到第 0 列
            g->pieceRow = LAST_ROW - 3; // 底边贴住最后一行
            break;

        default:
            break;
    }
}

// ---------- 走一手"不消行"的锁定，用来把连击清零 ----------
static inline void LockWithoutClear(GameState* g)
{
    ClearBoard(g);
    SetPieceAt(g, TETROMINO_O, -1, 0);   // 贴左边落下，只占 2 列，凑不满任何一行
    game_hard_drop(g);
}

#endif // TEST_BOARD_HELPERS_H

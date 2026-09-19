// ============================================================
// ui_board.c —— 棋盘区域绘制的实现
// ------------------------------------------------------------
// 所属层次：界面层
//
// 本文件只做"画"这件事。棋盘上每一格该不该有方块，由调用方（后续是逻辑层）
// 决定，这里不判断任何游戏规则。
// ============================================================
#include "ui_board.h"
#include "ui_config.h"
#include "tetromino.h"   // 真实形状数据（界面层只"取用"形状，不判断游戏规则）

// ============================================================
// 内部辅助函数：把"棋盘坐标"换算成"屏幕像素坐标"
// ------------------------------------------------------------
// 棋盘坐标 (col, row)：col 是列号，0 表示最左一列；row 是行号，0 表示最上面一行。
// 返回的是这一格左上角在窗口里的像素坐标。
// 之所以单独抽一个函数，是因为所有绘制都以它为准，改布局只需改 ui_config.h。
// ============================================================
static Vector2 BoardCellTopLeft(int col, int row)
{
    Vector2 p;
    p.x = (float)(BOARD_ORIGIN_X + col * CELL_SIZE);
    p.y = (float)(BOARD_ORIGIN_Y + row * CELL_SIZE);
    return p;
}

// ============================================================
// 画棋盘底色、网格线与外框
// ============================================================
void BoardDrawBackgroundAndGrid(void)
{
    // ---------- 1. 铺底色 ----------
    // 先整块铺深蓝底色，后面的网格线和方块都画在它上面。
    DrawRectangle(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_PIXEL_W, BOARD_PIXEL_H,
                  COLOR_BOARD_BG);

    // ---------- 2. 画竖网格线 ----------
    // 从第 1 列画到第 9 列（最左和最右由外框负责，不重复画）。
    // 每 5 格一条更亮的主网格线，用来帮助玩家快速数格子。
    for (int c = 1; c < BOARD_COLS; c++)
    {
        int x = BOARD_ORIGIN_X + c * CELL_SIZE;
        Color lineColor = (c % GRID_MAJOR_STEP == 0) ? COLOR_GRID_MAJOR : COLOR_GRID;
        DrawLine(x, BOARD_ORIGIN_Y, x, BOARD_ORIGIN_Y + BOARD_PIXEL_H, lineColor);
    }

    // ---------- 3. 画横网格线 ----------
    for (int r = 1; r < BOARD_VISIBLE_ROWS; r++)
    {
        int y = BOARD_ORIGIN_Y + r * CELL_SIZE;
        Color lineColor = (r % GRID_MAJOR_STEP == 0) ? COLOR_GRID_MAJOR : COLOR_GRID;
        DrawLine(BOARD_ORIGIN_X, y, BOARD_ORIGIN_X + BOARD_PIXEL_W, y, lineColor);
    }

    // ---------- 4. 画外框 ----------
    // 用霓虹青描一圈边框，把棋盘从深色背景里"框"出来。
    // 参数含义：(矩形, 线的粗细, 颜色)
    Rectangle frame = {
        (float)BOARD_ORIGIN_X, (float)BOARD_ORIGIN_Y,
        (float)BOARD_PIXEL_W,  (float)BOARD_PIXEL_H
    };
    DrawRectangleLinesEx(frame, 2.0f, COLOR_NEON);
}

// ============================================================
// 画一个实心方块格
// ------------------------------------------------------------
// 画法分两层：
//   1) 外发光：在格子外围画一个略大、低透明度的同色矩形，营造霓虹感；
//   2) 主体：在格子内部留 2 像素缝隙画实心矩形，让相邻格之间不粘成一片。
// 参数 col/row 超出棋盘范围时不画（防止越界画到面板上）。
// ============================================================
void BoardDrawBlockCell(int col, int row, Color color)
{
    if (col < 0 || col >= BOARD_COLS || row < 0 || row >= BOARD_VISIBLE_ROWS)
    {
        return; // 越界：直接不画
    }

    Vector2 p = BoardCellTopLeft(col, row);
    int x = (int)p.x;
    int y = (int)p.y;

    // 外发光：Fade 把颜色的透明度按比例调低（0.25 表示 25% 不透明）
    DrawRectangle(x - 1, y - 1, CELL_SIZE + 2, CELL_SIZE + 2, Fade(color, 0.25f));

    // 方块主体：四周各留 1 像素，于是相邻两格之间自然形成 2 像素缝隙
    DrawRectangle(x + 1, y + 1, CELL_SIZE - 2, CELL_SIZE - 2, color);
}

// ============================================================
// 画一个幽灵格（落点影子）
// ------------------------------------------------------------
// 幽灵块的作用是提前告诉玩家"现在硬降会落在哪里"，所以它必须：
//   · 看得见（有描边，轮廓清楚）；
//   · 不抢戏（填充非常淡，不能盖过真方块）。
// ============================================================
void BoardDrawGhostCell(int col, int row, Color color)
{
    if (col < 0 || col >= BOARD_COLS || row < 0 || row >= BOARD_VISIBLE_ROWS)
    {
        return;
    }

    Vector2 p = BoardCellTopLeft(col, row);
    Rectangle cell = { p.x + 1.0f, p.y + 1.0f,
                       (float)(CELL_SIZE - 2), (float)(CELL_SIZE - 2) };

    DrawRectangle((int)cell.x, (int)cell.y, CELL_SIZE - 2, CELL_SIZE - 2,
                  Fade(color, 0.16f));                       // 极淡的填充
    DrawRectangleLinesEx(cell, 1.5f, Fade(color, 0.55f));    // 半透明的轮廓
}

// ============================================================
// 按 4×4 盒子画一整块方块
// ------------------------------------------------------------
// 盒子里凡是为 1 的格子，就换算成棋盘坐标画一个方块格：
//     棋盘列 = originCol + 盒子列
//     棋盘行 = originRow + 盒子行
// 越出棋盘可见范围的格子会被 BoardDrawBlockCell 自己忽略，
// 所以方块"从顶部进场"时这里不需要写特殊处理。
// ============================================================
void BoardDrawTetromino(const int box[TETROMINO_BOX][TETROMINO_BOX],
                        int originCol, int originRow, Color color)
{
    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            if (box[r][c])
            {
                BoardDrawBlockCell(originCol + c, originRow + r, color);
            }
        }
    }
}

// ============================================================
// 按 4×4 盒子画一整块方块的幽灵（落点影子）
// ------------------------------------------------------------
// 与上面唯一的差别是改用幽灵格那种"低调"的画法。
// 本函数只负责"照着给的位置画"，落点算在哪里由调用方决定
// （Phase 5 起由游戏逻辑层算出真正的落点后传进来）。
// ============================================================
void BoardDrawTetrominoGhost(const int box[TETROMINO_BOX][TETROMINO_BOX],
                             int originCol, int originRow, Color color)
{
    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            if (box[r][c])
            {
                BoardDrawGhostCell(originCol + c, originRow + r, color);
            }
        }
    }
}

// ============================================================
// 按游戏状态绘制整个棋盘（Phase 5 起使用）
// ------------------------------------------------------------
// 这是棋盘绘制的总入口。它只做"读状态 → 画图"，不判断任何游戏规则：
// 哪些格子有方块、当前方块在哪、幽灵落在哪，全部由逻辑层负责算好。
// ============================================================
void BoardDrawGame(const GameState* game)
{
    // ---------- 1. 底色与网格 ----------
    BoardDrawBackgroundAndGrid();

    // ---------- 2. 已锁定的方块 ----------
    // 逻辑行 0~1 是隐藏缓冲行，不画；只画逻辑行 2~21（对应屏幕第 0~19 行）。
    for (int row = BOARD_BUFFER_ROWS; row < BOARD_TOTAL_ROWS; row++)
    {
        for (int col = 0; col < BOARD_COLS; col++)
        {
            const int id = game_board_at(game, row, col);
            if (id != BOARD_EMPTY)
            {
                BoardDrawBlockCell(col, row - BOARD_BUFFER_ROWS, BlockColorOf(id));
            }
        }
    }

    const Color pieceColor = BlockColorOf(game->pieceId);

    // ---------- 3. 消行闪烁 ----------
    // 正在消失的那几行铺一层亮色，随进度从亮到透明 —— 这就是规格里
    // "极短的闪烁淡出（约 0.15 秒），不做花哨动画"。
    if (game_is_clearing(game))
    {
        const double progress = game_clear_progress(game);          // 0 → 1
        const unsigned char alpha = (unsigned char)(220.0 * (1.0 - progress));
        const Color flash = { 255, 255, 255, alpha };

        for (int row = BOARD_BUFFER_ROWS; row < BOARD_TOTAL_ROWS; row++)
        {
            if (game_row_is_clearing(game, row))
            {
                DrawRectangle(BOARD_ORIGIN_X,
                              BOARD_ORIGIN_Y + (row - BOARD_BUFFER_ROWS) * CELL_SIZE,
                              BOARD_PIXEL_W, CELL_SIZE, flash);
            }
        }

        // 闪烁期间没有"当前方块"（上一块已锁定、下一块还没出），
        // 所以幽灵块与当前方块都不画 —— 否则会在旧位置留下一个不动的大方块。
        return;
    }

    // ---------- 4. 幽灵块 ----------
    // 先画幽灵：如果方块已经在落点上，就跳过不画，免得和真方块糊在一起。
    const int ghostRow = game_ghost_row(game);
    if (ghostRow != game->pieceRow)
    {
        BoardDrawTetrominoGhost(game->pieceBox, game->pieceCol,
                                ghostRow - BOARD_BUFFER_ROWS, pieceColor);
    }

    // ---------- 5. 当前方块 ----------
    BoardDrawTetromino(game->pieceBox, game->pieceCol,
                       game->pieceRow - BOARD_BUFFER_ROWS, pieceColor);
}

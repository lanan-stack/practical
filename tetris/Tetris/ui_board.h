// ============================================================
// ui_board.h —— 棋盘区域的绘制
// ------------------------------------------------------------
// 所属层次：界面层
//
// 职责：把"棋盘长什么样"画出来 —— 底色、网格线、外框、已锁定的方块、
//       正在下落的方块、幽灵块（落点影子）。
//
// 边界：本模块只负责"画"。棋盘上有什么、方块该在哪一格，一律由逻辑层
//       决定后告诉它，这里不做任何游戏规则判断。
// ============================================================
#ifndef UI_BOARD_H
#define UI_BOARD_H

#include "raylib.h"
#include "config.h"   // 需要 TETROMINO_BOX（4×4 盒子）这个常量
#include "game_logic.h" // 需要读取游戏状态（只读，界面层不修改游戏规则）

// ---------- 画棋盘底色、网格线与外框 ----------
// 参数：无
// 返回：无
// 说明：应在画任何方块之前调用，因为它会把整块棋盘底色铺满。
void BoardDrawBackgroundAndGrid(void);

// ---------- 画一个"实心方块格"（正在下落或已经锁定的方块）----------
// 参数：col 列号（0 在最左）、row 行号（0 在最上）、color 该方块的颜色
// 返回：无
// 说明：格与格之间留 2 像素缝隙，让方块有颗粒感；外圈带一层微弱发光。
void BoardDrawBlockCell(int col, int row, Color color);

// ---------- 画一个"幽灵格"（落点影子）----------
// 参数：col 列号、row 行号、color 对应方块的颜色
// 返回：无
// 说明：用低透明度的淡填充 + 描边，只提示位置，不能比真方块更抢眼。
void BoardDrawGhostCell(int col, int row, Color color);

// ---------- 按游戏状态绘制整个棋盘（Phase 5 起使用）----------
// 参数：game —— 游戏状态（只读）
// 返回：无
// 说明：一次画三样东西，顺序很重要：
//       1) 棋盘底色与网格；
//       2) 已锁定的方块（从逻辑棋盘读取内容，按方块编号上色）；
//       3) 幽灵块与前当方块 —— 幽灵先画，当前方块压在上面，
//          这样当方块已经落到幽灵位置时，看到的是真方块而不是影子。
// 行号换算：逻辑行（0~21）→ 屏幕行（0~19）= 逻辑行 - 隐藏缓冲行数。
void BoardDrawGame(const GameState* game);

// ---------- 按 4×4 盒子画一整块方块 ----------
// 参数：box       —— 形状数据（1 表示有方块），通常是 tetromino 模块给出的姿态
//       originCol —— 盒子左上角对准棋盘的列号
//       originRow —— 盒子左上角对准棋盘的行号（0 是最上面一行，可为负表示在可见区之上）
//       color     —— 方块颜色
// 返回：无
// 说明：遍历盒子的 16 格，凡是 1 就按棋盘坐标画一个格子；
//       落在可见区之外的格子会被 BoardDrawBlockCell 自动忽略（这在方块从顶部进场时很重要）。
void BoardDrawTetromino(const int box[TETROMINO_BOX][TETROMINO_BOX],
                        int originCol, int originRow, Color color);

// ---------- 按 4×4 盒子画一整块方块的幽灵（落点影子）----------
// 参数含义同 BoardDrawTetromino，只是改用幽灵格的低调画法。
void BoardDrawTetrominoGhost(const int box[TETROMINO_BOX][TETROMINO_BOX],
                             int originCol, int originRow, Color color);

#endif // UI_BOARD_H

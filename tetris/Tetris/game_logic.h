// ============================================================
// game_logic.h —— 游戏核心逻辑（逻辑层）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，**不依赖 raylib**，可独立单元测试）
//
// 职责（Phase 5 范围）：
//   · 棋盘状态（锁定方块）
//   · 碰撞检测
//   · 左右移动、旋转（不做贴墙补偿）
//   · 重力下落、软降、硬降
//   · 落地判定与锁定延迟（0.5 秒，最多重置 8 次）
//   · 幽灵落点计算
//   · 与袋子、暂存模块的联动，顶出判定
//
// 不负责（留给后续阶段）：消行、计分、连击、等级与难度、结束画面。
//
// 【重要设计约定：时间由调用方喂进来】
// 所有与时间有关的行为都通过 game_update(g, dt, softDrop) 推进，
// 而不是自己去读系统时间。这样测试可以精确控制"过了多少秒"，
// 手感相关的逻辑也能被单元测试覆盖。
// ============================================================
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>
#include "config.h"
#include "tetromino.h"
#include "bag.h"
#include "hold.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// 难度档位（依据 project.md 第 2.2 节）
// ------------------------------------------------------------
// 三档共用同一条速度曲线与升级节奏，只在"速度封顶在第几级"上分道扬镳：
//   轻松：5 级封顶（0.36 秒/格）—— 不会主动把玩家逼死
//   标准：8 级封顶（0.13 秒/格）
//   挑战：11 级封顶（0.043 秒/格）
// 注意：**等级本身不封顶**（继续显示真实等级），只有"下落速度"按封顶等级计算。
// ============================================================
typedef enum Difficulty
{
    DIFFICULTY_EASY = 0,     // 轻松
    DIFFICULTY_NORMAL = 1,   // 标准
    DIFFICULTY_HARD = 2,     // 挑战
    DIFFICULTY_COUNT = 3
} Difficulty;

// 一局游戏的全部状态
typedef struct GameState
{
    // ---------- 棋盘 ----------
    // 每一格：BOARD_EMPTY(-1) 表示空，0~6 表示被哪种方块占据（供着色用）
    int board[BOARD_TOTAL_ROWS][BOARD_COLS];

    // ---------- 当前方块 ----------
    int pieceId;                                  // 当前方块编号
    int pieceBox[TETROMINO_BOX][TETROMINO_BOX];   // 当前形状（含旋转姿态）
    int pieceCol;                                 // 4×4 盒子左上角所在列
    int pieceRow;                                 // 4×4 盒子左上角所在行（逻辑行）

    // ---------- 方块来源与暂存 ----------
    Bag  bag;
    Hold hold;

    // ---------- 时间与落地状态 ----------
    double fallTimer;       // 距离下一次自动下落累计了多久
    bool   onGround;        // 当前方块是否已经落地
    double lockTimer;       // 落地后已经过去多久（到 0.5 秒就锁定）
    int    lockResetCount;  // 锁定延迟被"成功的移动/旋转"重置过几次

    // ---------- 结束标记 ----------
    bool gameOver;          // 出生位置被占时为 true，此后不再产生新方块

    // ---------- 难度、计分与等级（Phase 6）----------
    Difficulty difficulty;  // 当前难度档
    int  level;             // 当前等级（= 1 + 累计消行数 / 10，不封顶）
    int  totalLines;        // 本局累计消行数
    int  score;             // 本局分数
    int  comboCount;        // 当前连击次数（连续多少次"落块后有消行"）

    // ---------- 消行闪烁阶段（Phase 6）----------
    // 检测到满行后不会立刻移除，而是先进入这个阶段：
    //   闪烁期间棋盘内容保持不变、不能操作、不产生新方块；
    //   时间到才真正把行移除、上方整体下移，然后出下一块。
    bool   isClearing;                       // 是否处于消行闪烁阶段
    double clearTimer;                       // 闪烁剩余时间
    int    clearingCount;                    // 这一次消掉几行
    bool   clearingRows[BOARD_TOTAL_ROWS];   // 哪几行（逻辑行）正在闪烁
} GameState;

// ---------- 初始化 / 销毁 ----------
// 参数：seed —— 袋子随机种子（同一个种子得到同一串方块，便于复现）
void game_init(GameState* g, unsigned int seed);
void game_destroy(GameState* g);

// ---------- 难度 ----------
// 设置难度档（越界值会被夹到合法范围）。开局默认是"标准"。
void game_set_difficulty(GameState* g, Difficulty difficulty);

// 取某一档的封顶等级（轻松 5 / 标准 8 / 挑战 11）
int difficulty_cap_level(Difficulty difficulty);

// 取当前下落间隔（秒/格）：按"当前等级"查速度表，并按难度封顶。
// 等级越高间隔越小（越快）；超过封顶等级后间隔不再变小。
double game_gravity_seconds(const GameState* g);

// ---------- 推进时间 ----------
// 参数：dt           —— 距上一帧经过的秒数
//       softDropHeld —— 是否按住软降键（按住时下落速度变为 10 倍）
// 说明：内部处理重力、落地判定、锁定延迟与锁定后的出新块。
void game_update(GameState* g, double dt, int softDropHeld);

// ---------- 碰撞检测（只读，可单独调用）----------
// 判断某个形状以 (col, row) 为盒子左上角时，是否与墙、地板或已锁定方块冲突。
// 返回：true = 可以放；false = 冲突
// 注意：隐藏缓冲行（逻辑 0~1 行）**允许**站立；逻辑行小于 0 视为越界。
bool game_can_place(const GameState* g, const int box[TETROMINO_BOX][TETROMINO_BOX],
                    int col, int row);

// ---------- 操作 ----------
bool game_move(GameState* g, int dx);                 // 左右移动一格
bool game_rotate(GameState* g, bool clockwise);       // 旋转（不做贴墙补偿）
void game_hard_drop(GameState* g);                    // 硬降：落到底并立即锁定
bool game_hold_piece(GameState* g);                   // 暂存

// ---------- 查询 ----------
// 幽灵落点：当前方块硬降后会停在哪一行（盒子左上角的行号）
int game_ghost_row(const GameState* g);

// 读取棋盘某一格的内容（供界面层绘制）；行列越界时返回 BOARD_EMPTY
int game_board_at(const GameState* g, int row, int col);

// ---------- 消行闪烁的查询接口（供界面层使用）----------
bool   game_is_clearing(const GameState* g);              // 是否正在消行闪烁
bool   game_row_is_clearing(const GameState* g, int row); // 某个逻辑行是否正在闪烁
double game_clear_progress(const GameState* g);           // 闪烁进度：0 刚进入 → 1 结束

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // GAME_LOGIC_H

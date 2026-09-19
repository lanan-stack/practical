// ============================================================
// game_logic.c —— 游戏核心逻辑的实现（Phase 5 部分）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib）
//
// 【时间推进的三条分支】game_update 是所有时间行为的入口：
//   ① 已结束          → 什么都不做
//   ② 方块在地面上    → 累计锁定计时，满 0.5 秒就锁定
//   ③ 方块悬在空中    → 累计下落计时，每满一个间隔就下移一格
// 这样"锁定延迟"和"重力"永远不会同时发生，逻辑不会打架。
// ============================================================
#include "game_logic.h"
#include <string.h>

// ============================================================
// 速度表与计分表（数值依据 project.md 第 2.2 / 2.3 节）
// ============================================================
// 第 1~11 级的下落间隔（秒/格）：等级越高，间隔越小（越快）。
// 至于"最高算到第几级"，由难度档决定（见 difficulty_cap_level）。
static const double LEVEL_GRAVITY_SECONDS[11] =
{
    1.00, 0.79, 0.62, 0.47, 0.36, 0.26, 0.19, 0.13, 0.094, 0.064, 0.043
};

// 一次消掉 1 / 2 / 3 / 4 行的基础分，下标就是消掉的行数
static const int LINE_CLEAR_POINTS[5] = { 0, 100, 300, 500, 800 };

// ============================================================
// 内部函数声明
// ============================================================
static void game_set_current(GameState* g, int id);
static void game_spawn_next(GameState* g);
static void game_lock_piece(GameState* g);
static void game_after_action(GameState* g);
static int  game_detect_full_rows(const GameState* g, bool rowsOut[BOARD_TOTAL_ROWS]);
static void game_apply_clear(GameState* g);

// ============================================================
// 初始化
// ------------------------------------------------------------
// 棋盘全部清空，袋子按种子初始化，然后放上第一块。
// ============================================================
void game_init(GameState* g, unsigned int seed)
{
    for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
    {
        for (int c = 0; c < BOARD_COLS; c++)
        {
            g->board[r][c] = BOARD_EMPTY;
        }
    }

    bag_init(&g->bag, seed);
    hold_init(&g->hold);

    g->pieceId = 0;
    tetromino_clear(g->pieceBox);
    g->pieceCol = SPAWN_BOX_COL;
    g->pieceRow = SPAWN_BOX_ROW;

    g->fallTimer = 0.0;
    g->onGround = false;
    g->lockTimer = 0.0;
    g->lockResetCount = 0;
    g->gameOver = false;

    // ---------- 难度与计分（Phase 6）----------
    // 默认"标准"档；Phase 7 会加开始界面让玩家自己选。
    g->difficulty = DIFFICULTY_NORMAL;
    g->level = 1;
    g->totalLines = 0;
    g->score = 0;
    g->comboCount = 0;

    g->isClearing = false;
    g->clearTimer = 0.0;
    g->clearingCount = 0;
    for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
    {
        g->clearingRows[r] = false;
    }

    // 取出第一块放到出生位置
    int id = 0;
    if (!bag_next(&g->bag, &id))
    {
        g->gameOver = true;
        return;
    }
    game_set_current(g, id);
}

void game_destroy(GameState* g)
{
    bag_destroy(&g->bag);
}

// ============================================================
// 碰撞检测
// ------------------------------------------------------------
// 对形状里每一个"有方块"的格子做三项检查：
//   1) 左右不能越出 0 ~ BOARD_COLS-1；
//   2) 上下不能越出逻辑棋盘（行 < 0 视为越界，行 ≥ 22 也越界）；
//   3) 目标格必须是空的。
// 任意一项不满足即冲突。
// ============================================================
bool game_can_place(const GameState* g, const int box[TETROMINO_BOX][TETROMINO_BOX],
                    int col, int row)
{
    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            if (!box[r][c])
            {
                continue;
            }

            const int bc = col + c;
            const int br = row + r;

            if (bc < 0 || bc >= BOARD_COLS)
            {
                return false;   // 撞左右墙
            }
            if (br < 0 || br >= BOARD_TOTAL_ROWS)
            {
                return false;   // 越出逻辑棋盘（br < 0 属防御性检查，正常玩不到）
            }
            if (g->board[br][bc] != BOARD_EMPTY)
            {
                return false;   // 压到已锁定的方块
            }
        }
    }
    return true;
}

// ============================================================
// 内部：把某个方块放到出生位置（姿态归零）
// ------------------------------------------------------------
// 三件事一起做，缺一不可：
//   1) 形状回到初始姿态（换出来的方块必须归零，不能保留之前的旋转）；
//   2) 位置回到出生点（列 3、行 1）；
//   3) 各种计时清零，并重新判断是否落地。
// 最后检查出生位置是否被占 —— 被占就是"顶出"，本局结束。
// ============================================================
static void game_set_current(GameState* g, int id)
{
    g->pieceId = id;
    tetromino_base_copy(id, g->pieceBox);
    g->pieceCol = SPAWN_BOX_COL;
    g->pieceRow = SPAWN_BOX_ROW;

    g->fallTimer = 0.0;
    g->lockTimer = 0.0;
    g->lockResetCount = 0;
    g->onGround = !game_can_place(g, g->pieceBox, g->pieceCol, g->pieceRow + 1);

    // 出生位置本身就被占住 → 顶出，本局结束
    if (!game_can_place(g, g->pieceBox, g->pieceCol, g->pieceRow))
    {
        g->gameOver = true;
    }
}

// ============================================================
// 内部：从袋子里取下一块并放到出生位置
// ============================================================
static void game_spawn_next(GameState* g)
{
    int nextId = 0;
    if (!bag_next(&g->bag, &nextId))
    {
        g->gameOver = true;
        return;
    }
    game_set_current(g, nextId);
}

// ============================================================
// 内部：找出所有被填满的行
// ------------------------------------------------------------
// 参数：rowsOut —— 输出数组，大小是逻辑行数；第 r 项表示第 r 行是否满
// 返回：满行的数量
// 说明：这一趟扫描同时也把"哪几行满"记了下来，后面移除和闪烁都要用。
// ============================================================
static int game_detect_full_rows(const GameState* g, bool rowsOut[BOARD_TOTAL_ROWS])
{
    int count = 0;

    for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
    {
        bool full = true;
        for (int c = 0; c < BOARD_COLS; c++)
        {
            if (g->board[r][c] == BOARD_EMPTY)
            {
                full = false;
                break;
            }
        }
        rowsOut[r] = full;
        if (full)
        {
            count++;
        }
    }
    return count;
}

// ============================================================
// 内部：锁定当前方块
// ------------------------------------------------------------
// 完整顺序（顺序本身就是规则，不能随意调整）：
//   1) 把方块写进棋盘；
//   2) 解锁暂存（本块已落地，下一块可以再换）；
//   3) 找出满行；
//   4) 有满行 → 立刻结算分数与等级，然后**进入消行闪烁阶段**（这时还不移除行）；
//      没有满行 → 连击归零，直接出下一块；
//   5) 真正移除行、上新方块，都发生在闪烁结束时（见 game_apply_clear）。
//
// 【为什么分数在这里结算、行却要等闪烁结束才移除】
// 分数和等级应该"一落地就立刻反映"，玩家才知道自己这一手值多少；
// 而行的消失需要一个 0.15 秒的视觉过程，所以两者分开。
// ============================================================
static void game_lock_piece(GameState* g)
{
    // ---------- 1. 写进棋盘 ----------
    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            if (g->pieceBox[r][c])
            {
                g->board[g->pieceRow + r][g->pieceCol + c] = g->pieceId;
            }
        }
    }

    // ---------- 2. 本块锁定了，下一块可以再暂存 ----------
    hold_mark_locked(&g->hold);

    // ---------- 3. 找满行 ----------
    bool fullRows[BOARD_TOTAL_ROWS];
    const int cleared = game_detect_full_rows(g, fullRows);

    if (cleared > 0)
    {
        // ---------- 4a. 计分与升级 ----------
        // 注意：用的是**消行前**的等级 —— 这一手是玩家用当时的等级完成的。
        const int levelBefore = g->level;

        int gained = LINE_CLEAR_POINTS[cleared] * levelBefore;

        g->comboCount++;   // 连击次数 +1（一次消 4 行也只算 1 次）
        if (g->comboCount >= 2)
        {
            // 连击加成：50 × (连击次数 − 1) × 消行前等级
            gained += COMBO_BONUS_BASE * (g->comboCount - 1) * levelBefore;
        }

        g->score += gained;
        g->totalLines += cleared;
        g->level = 1 + g->totalLines / LINES_PER_LEVEL;

        // ---------- 4b. 进入消行闪烁阶段 ----------
        g->isClearing = true;
        g->clearTimer = CLEAR_FLASH_SECONDS;
        g->clearingCount = cleared;
        for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
        {
            g->clearingRows[r] = fullRows[r];
        }
        return;   // 新方块要等闪烁结束才出
    }

    // ---------- 4c. 没有消行：连击归零，直接出下一块 ----------
    g->comboCount = 0;
    game_spawn_next(g);
}

// ============================================================
// 内部：闪烁结束 —— 真正移除满行、上方整体下移，然后出下一块
// ------------------------------------------------------------
// 下移用的是"压实"写法：从最底行往上扫，遇到要消的行就跳过，
// 其余的行依次搬到下面去。这样即使被消的行**不连续**（例如第 18、20 行满、
// 第 19 行没满）也能一次处理正确，而且保持原来的上下顺序不变。
//
// 注意顺序：**先消行、再出新块**。因为新块出生时要检查"出生位置是否被占"，
// 如果先判顶出，就会出现"本来能靠消行腾出空间、却被判游戏结束"的冤案。
// ============================================================
static void game_apply_clear(GameState* g)
{
    int writeRow = BOARD_TOTAL_ROWS - 1;

    for (int readRow = BOARD_TOTAL_ROWS - 1; readRow >= 0; readRow--)
    {
        if (g->clearingRows[readRow])
        {
            continue;   // 这一行要消失，跳过
        }

        if (writeRow != readRow)
        {
            for (int c = 0; c < BOARD_COLS; c++)
            {
                g->board[writeRow][c] = g->board[readRow][c];
            }
        }
        writeRow--;
    }

    // 上面空出来的行全部清空
    for (int r = writeRow; r >= 0; r--)
    {
        for (int c = 0; c < BOARD_COLS; c++)
        {
            g->board[r][c] = BOARD_EMPTY;
        }
    }

    // 收尾：退出闪烁状态
    g->isClearing = false;
    g->clearTimer = 0.0;
    g->clearingCount = 0;
    for (int r = 0; r < BOARD_TOTAL_ROWS; r++)
    {
        g->clearingRows[r] = false;
    }

    // 消行完成后再出新方块
    game_spawn_next(g);
}

// ============================================================
// 内部：一次"成功的操作"之后的收尾
// ------------------------------------------------------------
// 移动或旋转成功之后，方块的落地状态可能变了，分三种情况：
//   ① 结果悬空       → 锁定计时作废（避免"悬在半空还被锁死"），重新下落；
//   ② 本来就在地面   → 这是规格里的"锁定延迟重置"：清掉计时重新数，
//                      但最多只能重置 LOCK_RESET_MAX 次；
//   ③ 刚才还在空中、现在落地 → 锁定计时从头开始。
// 注意：只有**成功**的移动/旋转才会走到这里，撞墙的无效操作不享受重置
//       （用例 P5-15 就是守这一条的）。
// ============================================================
static void game_after_action(GameState* g)
{
    const bool wasOnGround = g->onGround;
    g->onGround = !game_can_place(g, g->pieceBox, g->pieceCol, g->pieceRow + 1);

    if (!g->onGround)
    {
        // 情况①：挪到悬空了
        g->lockTimer = 0.0;
        g->lockResetCount = 0;
        return;
    }

    if (!wasOnGround)
    {
        // 情况③：刚落地
        g->lockTimer = 0.0;
        g->lockResetCount = 0;
        return;
    }

    // 情况②：仍在地面上 —— 计入重置次数
    if (g->lockResetCount < LOCK_RESET_MAX)
    {
        g->lockTimer = 0.0;
        g->lockResetCount++;
    }
    // 重置次数用尽后：计时照常走，不再清零
}

// ============================================================
// 推进时间
// ============================================================
void game_update(GameState* g, double dt, int softDropHeld)
{
    if (g->gameOver)
    {
        return;   // 本局结束，时间不再推进
    }

    // ---------- 消行闪烁阶段：只倒计时，不落方块 ----------
    // 这段时间里棋盘保持原样（满行还看得见），到点才真正移除并出新块。
    if (g->isClearing)
    {
        g->clearTimer -= dt;
        if (g->clearTimer <= 0.0)
        {
            game_apply_clear(g);
        }
        return;
    }

    // 先刷新落地状态（方块可能因为上一帧的操作而变化）
    g->onGround = !game_can_place(g, g->pieceBox, g->pieceCol, g->pieceRow + 1);

    // ---------- 分支一：在地面上 → 走锁定延迟 ----------
    if (g->onGround)
    {
        g->lockTimer += dt;
        if (g->lockTimer >= LOCK_DELAY_SECONDS)
        {
            game_lock_piece(g);
        }
        return;
    }

    // ---------- 分支二：悬在空中 → 走重力 ----------
    // 下落间隔由"当前等级 + 难度封顶"共同决定（等级 1 时正好是 1.00 秒）。
    // 软降把它缩小到 1/10（也就是速度变成 10 倍）。
    const double baseInterval = game_gravity_seconds(g);
    const double interval = softDropHeld
                          ? (baseInterval / SOFT_DROP_MULTIPLIER)
                          : baseInterval;

    g->fallTimer += dt;

    // 用 while 而不是 if：一帧时间较长时（比如卡顿）可能要连续下移好几格
    while (g->fallTimer >= interval)
    {
        if (!game_can_place(g, g->pieceBox, g->pieceCol, g->pieceRow + 1))
        {
            g->fallTimer = 0.0;   // 落不下去了：余量丢掉，转为锁定计时
            break;
        }
        g->pieceRow++;
        g->fallTimer -= interval;
    }

    // 落下的过程中可能已经贴地，这时锁定计时从头开始数
    g->onGround = !game_can_place(g, g->pieceBox, g->pieceCol, g->pieceRow + 1);
    if (g->onGround)
    {
        g->lockTimer = 0.0;
        g->lockResetCount = 0;
    }
}

// ============================================================
// 左右移动
// ============================================================
bool game_move(GameState* g, int dx)
{
    if (g->gameOver)
    {
        return false;
    }
    if (!game_can_place(g, g->pieceBox, g->pieceCol + dx, g->pieceRow))
    {
        return false;   // 被挡住：状态一点都不改
    }

    g->pieceCol += dx;
    game_after_action(g);
    return true;
}

// ============================================================
// 旋转
// ------------------------------------------------------------
// 关键细节：先在**副本**上旋转并检测，只有确认放得下才写回当前形状。
// 这样"转不动"时形状和位置都保持原样，不会出现"转了一半"的坏状态。
// 本项目不做贴墙补偿：转不下就是转不下（规格里明确把踢墙留到以后扩展）。
// ============================================================
bool game_rotate(GameState* g, bool clockwise)
{
    if (g->gameOver)
    {
        return false;
    }

    int rotated[TETROMINO_BOX][TETROMINO_BOX];
    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            rotated[r][c] = g->pieceBox[r][c];
        }
    }

    if (clockwise)
    {
        tetromino_rotate_cw(rotated);
    }
    else
    {
        tetromino_rotate_ccw(rotated);
    }

    if (!game_can_place(g, rotated, g->pieceCol, g->pieceRow))
    {
        return false;   // 转不动：形状与位置都不变
    }

    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            g->pieceBox[r][c] = rotated[r][c];
        }
    }

    game_after_action(g);
    return true;
}

// ============================================================
// 硬降：一路向下直到不能再下，然后立即锁定（不等待延迟）
// ============================================================
void game_hard_drop(GameState* g)
{
    if (g->gameOver)
    {
        return;
    }

    while (game_can_place(g, g->pieceBox, g->pieceCol, g->pieceRow + 1))
    {
        g->pieceRow++;
    }
    game_lock_piece(g);
}

// ============================================================
// 暂存
// ------------------------------------------------------------
// hold 模块负责"编号怎么换、能不能换"，这里负责把换进来的方块
// 放到出生位置并归零姿态（用例 P5-22）。
// ============================================================
bool game_hold_piece(GameState* g)
{
    if (g->gameOver)
    {
        return false;
    }

    int currentId = g->pieceId;
    if (!hold_apply(&g->hold, &g->bag, &currentId))
    {
        return false;   // 本块已经换过一次，或取不到方块
    }

    game_set_current(g, currentId);
    return true;
}

// ============================================================
// 幽灵落点：从当前位置一路向下试，直到不能再下
// ------------------------------------------------------------
// 只读操作，不修改任何状态，可以随时调用（比如每帧画幽灵之前）。
// ============================================================
int game_ghost_row(const GameState* g)
{
    int row = g->pieceRow;
    while (game_can_place(g, g->pieceBox, g->pieceCol, row + 1))
    {
        row++;
    }
    return row;
}

// ============================================================
// 读取棋盘某一格（供界面层绘制）
// ============================================================
int game_board_at(const GameState* g, int row, int col)
{
    if (row < 0 || row >= BOARD_TOTAL_ROWS || col < 0 || col >= BOARD_COLS)
    {
        return BOARD_EMPTY;
    }
    return g->board[row][col];
}

// ============================================================
// 难度：封顶等级
// ------------------------------------------------------------
// 三档共用同一条速度曲线，区别只在"最高算到第几级"。
// ============================================================
int difficulty_cap_level(Difficulty difficulty)
{
    switch (difficulty)
    {
        case DIFFICULTY_EASY:   return 5;    // 轻松：5 级后不再变快
        case DIFFICULTY_HARD:   return 11;   // 挑战：11 级后不再变快
        case DIFFICULTY_NORMAL:
        default:                return 8;    // 标准：8 级后不再变快
    }
}

// ============================================================
// 难度：设置档位
// ------------------------------------------------------------
// 越界值夹到合法范围；下落速度是"用的时候现算"的（见 game_gravity_seconds），
// 所以这里改完立刻生效，不需要重算别的状态。
// ============================================================
void game_set_difficulty(GameState* g, Difficulty difficulty)
{
    if (difficulty < 0)
    {
        difficulty = DIFFICULTY_EASY;
    }
    if (difficulty >= DIFFICULTY_COUNT)
    {
        difficulty = DIFFICULTY_HARD;
    }
    g->difficulty = difficulty;
}

// ============================================================
// 当前下落间隔（秒/格）
// ------------------------------------------------------------
// 步骤：等级 → 按难度封顶 → 查速度表。
// 注意等级本身不封顶（界面上照常显示真实等级），只是查表时用封顶后的值，
// 这样玩家看得到自己的进度，速度又不会无限加快。
// ============================================================
double game_gravity_seconds(const GameState* g)
{
    const int cap = difficulty_cap_level(g->difficulty);

    int level = g->level;
    if (level < 1)
    {
        level = 1;
    }
    if (level > cap)
    {
        level = cap;
    }
    if (level > 11)
    {
        level = 11;   // 速度表只到 11 级，兜底防越界
    }

    return LEVEL_GRAVITY_SECONDS[level - 1];
}

// ============================================================
// 消行闪烁的查询接口（界面层每帧调用）
// ============================================================
bool game_is_clearing(const GameState* g)
{
    return g->isClearing;
}

bool game_row_is_clearing(const GameState* g, int row)
{
    if (row < 0 || row >= BOARD_TOTAL_ROWS)
    {
        return false;
    }
    return g->isClearing && g->clearingRows[row];
}

// 闪烁进度：0 表示刚开始闪，1 表示马上结束。
// 界面层拿它做淡出（越接近 1 越淡）。
double game_clear_progress(const GameState* g)
{
    if (!g->isClearing || CLEAR_FLASH_SECONDS <= 0.0)
    {
        return 1.0;
    }

    double progress = 1.0 - (g->clearTimer / CLEAR_FLASH_SECONDS);
    if (progress < 0.0) { progress = 0.0; }
    if (progress > 1.0) { progress = 1.0; }
    return progress;
}

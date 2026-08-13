// ============================================================
// 游戏逻辑层（P3 起使用）
// ------------------------------------------------------------
// 纯 C、不依赖 raylib，可独立单元测试。
// 负责：tick 移动、转向与输入缓冲（180° 拦截）、撞墙/撞自身
// 判定、吃食物增长、食物随机生成（避开蛇身）、计分与加速档位。
// 界面层只负责渲染与按键采集，不掺入任何游戏规则。
// ============================================================
#pragma once

#include "config.h"
#include "snake_list.h"

// 方向（数值设计成两两相隔 2 为相反方向：右<->左、下<->上）
typedef enum Direction
{
    DIR_RIGHT = 0,
    DIR_DOWN  = 1,
    DIR_LEFT  = 2,
    DIR_UP    = 3
} Direction;

// 一局游戏的全部状态
typedef struct GameState
{
    Snake snake;              // 蛇身（复用 P1 适配层）
    Direction dir;            // 当前移动方向

    // 输入缓冲队列（环形数组，容量见 SNAKE_INPUT_QUEUE_CAPACITY）
    Direction queue[SNAKE_INPUT_QUEUE_CAPACITY];
    int queueHead;            // 队头下标（下一次 tick 消费的位置）
    int queueCount;           // 队列中待消费的输入个数

    int foodRow;              // 食物行（-1 表示当前无食物）
    int foodCol;              // 食物列

    int score;                // 本局分数（每吃一个食物 +1）
    int speed;                // 当前速度（格/秒）
    int alive;                // 1=存活，0=已死亡

    unsigned int rngState;    // 内部随机数状态（可注入种子，便于测试复现）
} GameState;

// 初始化一局：蛇 3 格居中（尾(15,14)、身(15,15)、头(15,16)），
// 头朝右、速度 8、分数 0、存活，并生成第一个食物。
// seed 用于食物随机生成（同种子结果可复现，便于测试）。
void GameInit(GameState* g, unsigned int seed);

// 销毁：释放蛇链表等动态资源
void GameDestroy(GameState* g);

// 把方向加入输入缓冲（按 WASD 时由界面层调用）。
// 规则：拦截 180° 掉头（相对"当前方向或队列中最后一个方向"）；
// 队列已满时丢弃最旧输入。
void GameQueueDirection(GameState* g, Direction dir);

// 在棋盘随机空格上生成一个食物（不与蛇身重叠）。
// 若棋盘已被蛇占满则置 foodRow/foodCol 为 -1。
void GameSpawnFood(GameState* g);

// 推进一个 tick（界面层按 1/速度 秒调用一次）。
// 顺序：消费一个输入 → 算新头 → 撞墙判定 → 吃食物/尾巴跟上 →
// 撞自身判定 → 前进 → 吃食物结算（+1 分、加速、再生成食物）。
void GameTick(GameState* g);

// ---------- 只读查询（内联，方便界面层与测试直接使用）----------
static inline int GameScore(const GameState* g)       { return g->score; }
static inline int GameSpeed(const GameState* g)       { return g->speed; }
static inline int GameIsAlive(const GameState* g)     { return g->alive; }
static inline Direction GameDirection(const GameState* g) { return g->dir; }

// 加速档位：每吃满 5 个食物一档（上限 = 最大速度-初始速度 = 10 档）
static inline int GameSpeedLevel(const GameState* g)
{
    int level = g->score / SNAKE_FOODS_PER_LEVEL;
    int maxLevel = (SNAKE_MAX_SPEED - SNAKE_INIT_SPEED);
    return level > maxLevel ? maxLevel : level;
}

// 加速进度条值：本档位已吃食物数，范围 0..4（满 5 升档并归零）
static inline int GameFoodProgress(const GameState* g)
{
    return g->score % SNAKE_FOODS_PER_LEVEL;
}

// ---------- 最高分存档（P6）----------

// 从文件读取最高分。
// 规则：文件不存在、打不开、是目录、内容为空或只有空白、整个文件不是
// "单个十进制非负整数"（允许首尾空白/换行；+123、前导零也算合法整数形式）、
// 数值超出 int 范围、数字后还有任何内容（含第二行）、文件超过 4096 字节时，
// 一律返回 0（视为无存档）。
int HighScoreLoad(const char* path);

// 把最高分写入文件（覆盖写）。
// 成功返回 1，失败返回 0；path 为 NULL 或 score 为负数时直接失败。
int HighScoreSave(const char* path, int score);

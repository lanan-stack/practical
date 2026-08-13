// ============================================================
// 游戏逻辑层实现（纯 C，不依赖 raylib，可独立单元测试）
// ============================================================
#include "game_logic.h"
#include <stdio.h>    // fopen_s / fgets / fprintf（最高分存档）
#include <stdlib.h>   // strtol（最高分解析）
#include <errno.h>    // ERANGE（整数溢出检测）
#include <limits.h>   // INT_MAX

// 各方向的移动增量（与 Direction 枚举顺序对应：右/下/左/上）
static int DirDeltaRow(Direction d) { static const int dr[4] = { 0, 1, 0, -1 }; return dr[d]; }
static int DirDeltaCol(Direction d) { static const int dc[4] = { 1, 0, -1, 0 }; return dc[d]; }

// 相反方向：右↔左、下↔上
static Direction Opposite(Direction d) { return (Direction)((d + 2) % 4); }

// 简单线性同余随机数：可复现（同种子同序列），足够游戏用
static unsigned int NextRng(unsigned int* state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

void GameInit(GameState* g, unsigned int seed)
{
    g->rngState = seed != 0u ? seed : 1u;
    g->dir = DIR_RIGHT;
    g->queueHead = 0;
    g->queueCount = 0;
    g->foodRow = g->foodCol = -1;
    g->score = 0;
    g->speed = SNAKE_INIT_SPEED;
    g->alive = 1;

    // 开局蛇 3 格：先插尾巴再插身体最后插头（头插法，顺序反过来）
    SnakeInit(&g->snake);
    SnakePushHead(&g->snake, SnakePosEncode(SNAKE_START_HEAD_ROW, SNAKE_START_HEAD_COL - 2)); // 尾
    SnakePushHead(&g->snake, SnakePosEncode(SNAKE_START_HEAD_ROW, SNAKE_START_HEAD_COL - 1)); // 身
    SnakePushHead(&g->snake, SnakePosEncode(SNAKE_START_HEAD_ROW, SNAKE_START_HEAD_COL));     // 头

    GameSpawnFood(g); // 开局立即生成第一个食物
}

void GameDestroy(GameState* g)
{
    SnakeDestroy(&g->snake);
}

void GameQueueDirection(GameState* g, Direction dir)
{
    if (!g->alive) return; // 已死亡不接受输入

    // "当前生效方向" = 队列中最后一个方向（队列空则用当前移动方向）
    Direction effective = g->dir;
    if (g->queueCount > 0)
    {
        int last = (g->queueHead + g->queueCount - 1) % SNAKE_INPUT_QUEUE_CAPACITY;
        effective = g->queue[last];
    }

    if (dir == Opposite(effective)) return; // 拦截 180° 掉头

    if (g->queueCount == SNAKE_INPUT_QUEUE_CAPACITY)
    {
        // 队列已满：丢弃最旧输入（队头前进一格）
        g->queueHead = (g->queueHead + 1) % SNAKE_INPUT_QUEUE_CAPACITY;
        g->queueCount--;
    }

    int tail = (g->queueHead + g->queueCount) % SNAKE_INPUT_QUEUE_CAPACITY;
    g->queue[tail] = dir;
    g->queueCount++;
}

void GameSpawnFood(GameState* g)
{
    // 空格数 = 总格数 - 蛇身长度；随机挑第 roll 个空格
    int freeCells = SNAKE_BOARD_SIZE * SNAKE_BOARD_SIZE - SnakeLength(&g->snake);
    if (freeCells <= 0) // 棋盘被蛇占满：无食物可放
    {
        g->foodRow = g->foodCol = -1;
        return;
    }

    int roll = (int)(NextRng(&g->rngState) % (unsigned int)freeCells);
    int count = 0;
    for (int r = 0; r < SNAKE_BOARD_SIZE; r++)
    {
        for (int c = 0; c < SNAKE_BOARD_SIZE; c++)
        {
            if (!SnakeContains(&g->snake, SnakePosEncode(r, c)))
            {
                if (count == roll)
                {
                    g->foodRow = r;
                    g->foodCol = c;
                    return;
                }
                count++;
            }
        }
    }

    // 理论到不了这里（freeCells>0 必有空格），兜底置 -1
    g->foodRow = g->foodCol = -1;
}

void GameTick(GameState* g)
{
    if (!g->alive) return; // 死亡后 tick 无操作

    // 1. 消费一个排队输入（若有）
    if (g->queueCount > 0)
    {
        Direction next = g->queue[g->queueHead];
        g->queueHead = (g->queueHead + 1) % SNAKE_INPUT_QUEUE_CAPACITY;
        g->queueCount--;
        // 防御性再拦一次（正常流程入队时已拦截，不会触发）
        if (next != Opposite(g->dir))
        {
            g->dir = next;
        }
    }

    // 2. 计算新蛇头坐标
    int headRow = SnakePosRow(SnakeHead(&g->snake));
    int headCol = SnakePosCol(SnakeHead(&g->snake));
    int newRow = headRow + DirDeltaRow(g->dir);
    int newCol = headCol + DirDeltaCol(g->dir);

    // 3. 撞墙判定：越界即死，蛇身保持不变
    if (newRow < 0 || newRow >= SNAKE_BOARD_SIZE ||
        newCol < 0 || newCol >= SNAKE_BOARD_SIZE)
    {
        g->alive = 0;
        return;
    }

    int newPos = SnakePosEncode(newRow, newCol);
    int eating = (g->foodRow == newRow && g->foodCol == newCol);

    // 4. 撞自身判定：普通移动时尾巴格本 tick 会腾空，头进入尾格合法；
    //    其余任何蛇身格都算撞死。判定先于蛇身改动，
    //    保证"撞死"时蛇身保持原样（便于界面层定格展示）。
    if (SnakeContains(&g->snake, newPos))
    {
        if (eating)
        {
            g->alive = 0; // 吃食物时尾巴不动，撞到任何身体格必死
            return;
        }
        int tailPos = SnakeAt(&g->snake, SnakeLength(&g->snake) - 1);
        if (newPos != tailPos)
        {
            g->alive = 0; // 撞到中间身体（非尾巴）
            return;
        }
        // newPos 就是尾巴格：本 tick 尾巴腾空，合法，继续
    }

    // 5. 普通移动弹掉尾巴（尾巴腾空）；吃食物则不弹尾（增长一格）
    if (!eating)
    {
        SnakePopTail(&g->snake);
    }

    // 6. 头插前进
    SnakePushHead(&g->snake, newPos);

    // 7. 吃食物结算：+1 分、满 5 个加速一档（上限 18）、再生成新食物
    if (eating)
    {
        g->score++;
        if (g->score % SNAKE_FOODS_PER_LEVEL == 0 && g->speed < SNAKE_MAX_SPEED)
        {
            g->speed++;
        }
        GameSpawnFood(g);
    }
}

// ---------- 最高分存档（纯 C，路径由界面层传入，便于单元测试）----------

int HighScoreLoad(const char* path)
{
    if (path == NULL) return 0;

    FILE* fp = NULL;
    if (fopen_s(&fp, path, "r") != 0 || fp == NULL) return 0; // 文件不存在/打不开/是目录

    // 读完整文件（上限 4096 字节；超长一律视为非法内容）
    char buf[4096];
    size_t total = 0;
    int truncated = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF)
    {
        if (total + 1 < sizeof(buf)) buf[total++] = (char)ch;
        else truncated = 1;
    }
    fclose(fp);
    if (truncated) return 0;
    buf[total] = '\0';

    // 去掉首尾空白（空格/Tab/回车/换行）
    char* start = buf;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') start++;
    char* end = buf + total;
    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) end--;
    *end = '\0';

    if (*start == '\0') return 0; // 空文件或只有空白

    // 严格解析：整个文件必须恰好是一个十进制整数
    errno = 0;
    char* parseEnd = NULL;
    long value = strtol(start, &parseEnd, 10);
    if (errno == ERANGE || parseEnd == start) return 0; // 溢出或不是数字
    if (*parseEnd != '\0') return 0;                    // 数字后还有任何字符（含第二行）
    if (value < 0 || value > INT_MAX) return 0;         // 负数或超出 int
    return (int)value;
}

int HighScoreSave(const char* path, int score)
{
    if (path == NULL || score < 0) return 0;

    FILE* fp = NULL;
    if (fopen_s(&fp, path, "w") != 0 || fp == NULL) return 0; // 打不开/路径非法

    int written = fprintf(fp, "%d", score);
    int closed = fclose(fp);
    return (written > 0 && closed == 0) ? 1 : 0;
}

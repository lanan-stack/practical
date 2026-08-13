// ============================================================
// 蛇身适配层：把"带头双向循环链表 DCList"包装成贪吃蛇的数据结构
// ------------------------------------------------------------
// 设计要点：
//   1. 蛇身每节存一个 int 编码的坐标（行 * 棋盘边长 + 列），
//      这样不需要改动原 DCList 的 DCLDataType(int)。
//   2. 链表头节点不存数据，list_length 存蛇身长度。
//   3. 本层只封装链表操作，不含任何游戏规则（移动/碰撞等逻辑
//      在 P3 的游戏逻辑层实现）。
// ============================================================
#pragma once

#include "config.h"
#include "DCList.h"

// 蛇身单节坐标的 int 编码（row * SNAKE_BOARD_SIZE + col）
typedef int SnakePos;

// 表示"无效坐标"的取值（越界访问蛇身时返回）
#define SNAKE_INVALID_POS (-1)

// 蛇结构：内部就是一个带头双向循环链表
typedef struct Snake
{
    DCListNode* nodes; // 链表头节点（不存数据，list_length 记录蛇身长度）
} Snake;

// 坐标编解码（内联，方便逻辑层直接使用）
static inline SnakePos SnakePosEncode(int row, int col) { return row * SNAKE_BOARD_SIZE + col; }
static inline int      SnakePosRow(SnakePos pos)        { return pos / SNAKE_BOARD_SIZE; }
static inline int      SnakePosCol(SnakePos pos)        { return pos % SNAKE_BOARD_SIZE; }

// 初始化空蛇（只创建链表头节点）
void SnakeInit(Snake* snake);

// 销毁整条蛇（释放所有节点，并把指针置空）
void SnakeDestroy(Snake* snake);

// 蛇头方向插入一节（新蛇头）
void SnakePushHead(Snake* snake, SnakePos pos);

// 删除蛇尾一节（移动时"尾巴跟上"），空蛇时安全无操作
void SnakePopTail(Snake* snake);

// 返回蛇身长度（数据节点个数）
int SnakeLength(const Snake* snake);

// 返回蛇头坐标；空蛇返回 SNAKE_INVALID_POS
SnakePos SnakeHead(const Snake* snake);

// 返回第 index 节坐标（0 = 蛇头）；越界返回 SNAKE_INVALID_POS
SnakePos SnakeAt(const Snake* snake, int index);

// 判断某格是否被蛇身占用（1=占用，0=未占用）
int SnakeContains(const Snake* snake, SnakePos pos);

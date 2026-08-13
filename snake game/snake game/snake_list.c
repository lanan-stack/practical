// ============================================================
// 蛇身适配层实现（纯 C，不依赖 raylib，可独立单元测试）
// ============================================================
#include "snake_list.h"

// 说明：原 DCList.h 没有声明 DCLPopBack，但 DCList.c 中已实现。
// 为遵守"不改动链表源码"的约定，这里仅补一句外部声明，
// 以便用 O(1) 的尾删支持蛇的移动（头插 + 尾删）。
void DCLPopBack(DCListNode* DCL);

void SnakeInit(Snake* snake)
{
    snake->nodes = NULL;
    DCListInit1(&snake->nodes); // 创建链表头节点（自环）
}

void SnakeDestroy(Snake* snake)
{
    if (snake->nodes != NULL)
    {
        DCListDestory(&snake->nodes); // 释放全部节点并置空
    }
    snake->nodes = NULL;
}

void SnakePushHead(Snake* snake, SnakePos pos)
{
    DCLPushFront(snake->nodes, pos); // 头插：新蛇头插到头节点之后
}

void SnakePopTail(Snake* snake)
{
    if (SnakeLength(snake) > 0)
    {
        DCLPopBack(snake->nodes); // 尾删：删除最后一个数据节点
    }
    // 空蛇时什么都不做（避免触发原链表的"链表为空"打印分支）
}

int SnakeLength(const Snake* snake)
{
    return snake->nodes->list_length; // 长度存在头节点上
}

SnakePos SnakeHead(const Snake* snake)
{
    if (SnakeLength(snake) <= 0)
    {
        return SNAKE_INVALID_POS;
    }
    return snake->nodes->next->data; // 头节点后的第一个节点即蛇头
}

SnakePos SnakeAt(const Snake* snake, int index)
{
    if (index < 0 || index >= SnakeLength(snake))
    {
        return SNAKE_INVALID_POS;
    }
    DCListNode* cur = snake->nodes->next; // 从蛇头开始
    for (int i = 0; i < index; i++)
    {
        cur = cur->next;
    }
    return cur->data;
}

int SnakeContains(const Snake* snake, SnakePos pos)
{
    DCListNode* cur = snake->nodes->next; // 遍历所有数据节点
    while (cur != snake->nodes)
    {
        if (cur->data == pos)
        {
            return 1; // 找到占用
        }
        cur = cur->next;
    }
    return 0; // 未被占用
}

// ============================================================
// 全局配置（P1 起使用）
// 集中管理棋盘等可调参数，后续 Phase 的速度/颜色也收进这里
// ============================================================
#pragma once

// 棋盘为正方形，边长 30 格（坐标范围 0..29）
#define SNAKE_BOARD_SIZE 30

// ---------- 速度模型（格/秒）----------
#define SNAKE_INIT_SPEED 8      // 初始速度
#define SNAKE_MAX_SPEED 18      // 速度上限
#define SNAKE_FOODS_PER_LEVEL 5 // 每吃满几个食物加速一档

// ---------- 输入缓冲 ----------
#define SNAKE_INPUT_QUEUE_CAPACITY 3 // 转向排队容量（超出丢弃最旧）

// ---------- 开局蛇 ----------
#define SNAKE_START_HEAD_ROW 15 // 蛇头行
#define SNAKE_START_HEAD_COL 16 // 蛇头列（头朝右，出生在棋盘中央）

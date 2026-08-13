// ============================================================
// 界面层配置（P2 起使用）
// ------------------------------------------------------------
// 集中管理窗口尺寸、棋盘布局与霓虹配色，后续 Phase 调优
// 只需改这里。注意：本文件依赖 raylib（Color 类型），仅供
// 界面层使用；游戏逻辑层（snake_list 等）不得 include 本文件。
// ============================================================
#pragma once

#include "raylib.h"
#include "config.h" // SNAKE_BOARD_SIZE

// ---------- 窗口与棋盘布局 ----------
#define SCREEN_WIDTH 900                            // 窗口宽度
#define SCREEN_HEIGHT 700                           // 窗口高度
#define CELL_SIZE 18                                // 每个棋盘格的像素边长
#define BOARD_PIXELS (CELL_SIZE * SNAKE_BOARD_SIZE) // 棋盘总像素边长（30*18=540）
#define BOARD_ORIGIN_X ((SCREEN_WIDTH - BOARD_PIXELS) / 2) // 棋盘左上角 x（水平居中）
#define BOARD_ORIGIN_Y 95                           // 棋盘左上角 y（顶部留标题空间）

// ---------- 霓虹配色（与 project.md 视觉规格一致）----------
static const Color WINDOW_BG_COLOR = { 5, 8, 18, 255 };     // 窗口底色（近黑深蓝，衬托霓虹）
static const Color BOARD_BG_COLOR = { 11, 16, 48, 255 };    // 棋盘深蓝 #0B1030
static const Color GRID_COLOR = { 30, 42, 90, 255 };        // 网格线淡蓝 #1E2A5A
static const Color GRID_MAJOR_COLOR = { 48, 68, 138, 255 }; // 每 5 格一条的主网格线（稍亮）
static const Color NEON_COLOR = { 53, 197, 240, 255 };      // 霓虹青 #35C5F0（棋盘描边）
static const Color CAPTION_COLOR = { 61, 255, 138, 255 };   // 霓虹绿 #3DFF8A（标题文字）
static const Color HINT_COLOR = { 160, 180, 220, 255 };     // 提示文字（柔和浅蓝灰）

// ---------- P4 起：蛇 / 食物 / 死亡提示配色 ----------
static const Color SNAKE_BODY_COLOR = { 61, 255, 138, 255 }; // 霓虹绿 #3DFF8A（蛇身）
static const Color SNAKE_HEAD_COLOR = { 255, 210, 63, 255 }; // 亮黄 #FFD23F（蛇头）
static const Color EYE_WHITE_COLOR = { 255, 255, 255, 255 }; // 眼白
static const Color EYE_PUPIL_COLOR = { 18, 20, 40, 255 };    // 瞳孔（深色）
static const Color DEAD_TEXT_COLOR = { 255, 77, 77, 255 };   // 死亡提示红 #FF4D4D

// 食物七彩色（与 project.md 视觉规格一致）
static const Color FOOD_PALETTE[7] =
{
    { 255, 77, 77, 255 },   // 红 #FF4D4D
    { 255, 154, 61, 255 },  // 橙 #FF9A3D
    { 255, 217, 61, 255 },  // 黄 #FFD93D
    { 61, 220, 90, 255 },   // 绿 #3DDC5A
    { 53, 197, 240, 255 },  // 青 #35C5F0
    { 77, 107, 254, 255 },  // 蓝 #4D6BFE
    { 177, 77, 255, 255 }   // 紫 #B14DFF
};

// 中文字体路径（相对 exe 所在目录，构建后自动复制到 exe 旁）
#define FONT_PATH "resources/simhei.ttf"

// ============================================================
// 贪吃蛇 · 游戏入口（P6 阶段：完整流程）
// ------------------------------------------------------------
// 本阶段在 P5 基础上补齐完整闭环：
//   - 标题画面：中文"贪吃蛇" + 最高分 + 点击/Enter 开始
//   - 死亡弹窗：本局分数 + 历史最高分 + 重开/退出（含"新纪录"提示）
//   - 最高分本地存档：exe 同级 highscore.txt，重启后仍保留
//   - 音效：eat.wav（吃食物）/ death.wav（死亡）
//   - 右侧 HUD：分数 / 速度 / 档位 / 5 格加速进度条
//   - 保留：WASD/方向键物理轮询转向、四角星芒食物、
//     蛇身随食物变色（蛇头亮黄不变）、ESC 暂停菜单
// ============================================================

// ---------- 最小 Win32 声明 ----------
// 本文件只用到 GetAsyncKeyState（物理键盘状态轮询，不依赖窗口焦点）。
// 不引入整个 windows.h，避免与 raylib.h 的同名符号冲突
// （Rectangle / CloseWindow / ShowCursor / LoadImageW）。
extern short __stdcall GetAsyncKeyState(int vKey);
#pragma comment(lib, "user32.lib")

#define VK_UP      0x26
#define VK_DOWN    0x28
#define VK_LEFT    0x25
#define VK_RIGHT   0x27
#define VK_ESCAPE  0x1B
#define VK_RETURN  0x0D

#include <stdbool.h>
#include <stdio.h>   // snprintf（HUD / 弹窗 / 存档路径拼串）
#include <math.h>    // cosf / sinf（五角星顶点）
#include <time.h>    // 用当前时间做随机种子
#include "raylib.h"
#include "config.h"
#include "ui_config.h"
#include "game_logic.h"

// ---------- 界面文字 ----------
static const char *CAPTION_TEXT = "完整流程版（P6）";
static const char *HINT_TEXT = "WASD / 方向键 转向 | ESC 暂停";
static const char *PAUSE_TITLE_TEXT = "已暂停";
static const char *PAUSE_OPTION_TEXT[3] = { "继续", "重开", "退出" };
static const char *TITLE_TEXT = "贪吃蛇";
static const char *TITLE_START_HINT = "点击开始 或 按 Enter 开始";
static const char *DEATH_TITLE_TEXT = "游戏结束";
static const char *DEATH_OPTION_TEXT[2] = { "重开", "退出" };
static const char *NEW_RECORD_TEXT = "新纪录！";

// 字体图集需要覆盖的全部字符（所有界面文字合起来）
static const char *FONT_GLYPHS =
    "贪吃蛇完整流程版（P6）游戏结束已暂停最高分新纪录点击开始或按Enter"
    "WASD/方向键转向ESC暂停继续重开退出本局分数速度档位格秒加速进度"
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

// 加载包含指定中文字符的字体
// raylib 默认字体不含中文，必须显式告诉它需要哪些字形
static Font LoadChineseFont(int fontSize)
{
    int count = 0;
    int *codepoints = LoadCodepoints(FONT_GLYPHS, &count); // 把中文字符串转成字形编码
    Font font = LoadFontEx(FONT_PATH, fontSize, codepoints, count);
    UnloadCodepoints(codepoints); // 字形编码数组用完即释放
    return font;
}

// ---------- 棋盘 ----------

// 画霓虹描边：多层半透明外扩线 + 最内实色亮线，模拟发光效果
static void DrawNeonBorder(Rectangle rect, Color core)
{
    // 4 层光晕，从外到内逐层变亮；每层向外扩 4 像素
    for (int i = 1; i <= 4; i++)
    {
        float grow = i * 4.0f;
        Rectangle glowRect = { rect.x - grow, rect.y - grow,
                               rect.width + 2.0f * grow, rect.height + 2.0f * grow };
        Color glow = core;
        glow.a = (unsigned char)(6 + i * 8); // 外层最淡（14），内层渐亮（38）
        DrawRectangleLinesEx(glowRect, 1.5f, glow);
    }
    // 最内圈：2 像素实色亮边
    DrawRectangleLinesEx(rect, 2.0f, core);
}

// 画 30×30 棋盘：底色 + 网格线 + 霓虹边框
static void DrawBoard(void)
{
    Rectangle boardRect = { BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_PIXELS, BOARD_PIXELS };

    // 1. 棋盘深蓝底色
    DrawRectangleRec(boardRect, BOARD_BG_COLOR);

    // 2. 网格线：每 5 格一条主网格线（稍亮），其余为细网格线
    //    +0.5 让 1px 线落在像素中心，避免发虚
    for (int i = 0; i <= SNAKE_BOARD_SIZE; i++)
    {
        float x = BOARD_ORIGIN_X + (float)(i * CELL_SIZE);
        float y = BOARD_ORIGIN_Y + (float)(i * CELL_SIZE);
        Color lineColor = (i % 5 == 0) ? GRID_MAJOR_COLOR : GRID_COLOR;

        DrawLineV((Vector2){ x + 0.5f, BOARD_ORIGIN_Y },
                  (Vector2){ x + 0.5f, BOARD_ORIGIN_Y + BOARD_PIXELS }, lineColor);
        DrawLineV((Vector2){ BOARD_ORIGIN_X, y + 0.5f },
                  (Vector2){ BOARD_ORIGIN_X + BOARD_PIXELS, y + 0.5f }, lineColor);
    }

    // 3. 霓虹边框（最后画，盖在网格线上）
    DrawNeonBorder(boardRect, NEON_COLOR);
}

// ---------- 蛇与食物 ----------

// 取某格在窗口中的像素矩形
static Rectangle CellRect(int row, int col)
{
    return (Rectangle){ (float)(BOARD_ORIGIN_X + col * CELL_SIZE),
                        (float)(BOARD_ORIGIN_Y + row * CELL_SIZE),
                        CELL_SIZE, CELL_SIZE };
}

// 画蛇头眼睛：两只白眼球 + 瞳孔，位置随移动方向转动
static void DrawEyes(GameState* g, Rectangle cell)
{
    float cx = cell.x + cell.width / 2.0f;
    float cy = cell.y + cell.height / 2.0f;

    // 朝向单位向量（ex, ey）
    float ex = 0.0f, ey = 0.0f;
    switch (GameDirection(g))
    {
        case DIR_RIGHT: ex = 1.0f; break;
        case DIR_LEFT:  ex = -1.0f; break;
        case DIR_DOWN:  ey = 1.0f; break;
        case DIR_UP:    ey = -1.0f; break;
        default: break;
    }

    // 眼睛中心 = 朝向前移 + 向两侧张开（垂直向量为 (-ey, ex)）
    float fwd = CELL_SIZE * 0.18f;
    float side = CELL_SIZE * 0.18f;
    Vector2 sideVec = { -ey, ex };
    Vector2 eye1 = { cx + ex * fwd + sideVec.x * side, cy + ey * fwd + sideVec.y * side };
    Vector2 eye2 = { cx + ex * fwd - sideVec.x * side, cy + ey * fwd - sideVec.y * side };

    float eyeR = CELL_SIZE * 0.13f;
    float pupilR = CELL_SIZE * 0.06f;
    DrawCircleV(eye1, eyeR, EYE_WHITE_COLOR);
    DrawCircleV(eye2, eyeR, EYE_WHITE_COLOR);
    // 瞳孔略朝移动方向偏移，显得"在看前面"
    DrawCircleV((Vector2){ eye1.x + ex * pupilR, eye1.y + ey * pupilR }, pupilR, EYE_PUPIL_COLOR);
    DrawCircleV((Vector2){ eye2.x + ex * pupilR, eye2.y + ey * pupilR }, pupilR, EYE_PUPIL_COLOR);
}

// 画整条蛇：先画身体（从尾到头），最后画蛇头（盖在最上层）
// bodyColor：蛇身当前颜色（吃到什么颜色食物就变什么颜色；蛇头始终亮黄）
static void DrawSnake(GameState* g, Color bodyColor)
{
    int len = SnakeLength(&g->snake);

    for (int i = len - 1; i >= 1; i--)
    {
        int pos = SnakeAt(&g->snake, i);
        Rectangle r = CellRect(SnakePosRow(pos), SnakePosCol(pos));
        // 四周留 2px 间隙 + 圆角，呈现"一节一节"的霓虹方块感
        Rectangle seg = { r.x + 2.0f, r.y + 2.0f, r.width - 4.0f, r.height - 4.0f };
        DrawRectangleRounded(seg, 0.25f, 8, bodyColor);
    }

    // 蛇头：独立配色（亮黄），比身体略大一圈更醒目
    int headPos = SnakeHead(&g->snake);
    Rectangle hr = CellRect(SnakePosRow(headPos), SnakePosCol(headPos));
    Rectangle headRect = { hr.x + 1.0f, hr.y + 1.0f, hr.width - 2.0f, hr.height - 2.0f };
    DrawRectangleRounded(headRect, 0.3f, 8, SNAKE_HEAD_COLOR);
    DrawEyes(g, hr);
}

// 生成四角星芒顶点：8 个点交替长芒/短谷，绕中心每 45° 一个（闪光/星光造型）
static void BuildStarPoints(Vector2 center, float outerR, float innerR, Vector2* pts)
{
    for (int i = 0; i < 8; i++)
    {
        float radius = (i % 2 == 0) ? outerR : innerR;
        float angle = -PI / 2.0f + i * PI / 4.0f; // 长芒朝上
        pts[i].x = center.x + cosf(angle) * radius;
        pts[i].y = center.y + sinf(angle) * radius;
    }
}

// 画一颗四角星芒装饰（与食物同款可靠绕向，用于标题等装饰）
static void DrawSparkle(Vector2 center, float outerR, Color color)
{
    Vector2 pts[8];
    BuildStarPoints(center, outerR, outerR * 0.31f, pts);
    for (int i = 0; i < 8; i++)
    {
        DrawTriangle(pts[(i + 1) % 8], pts[i], center, color);
    }
}

// 食物颜色：由食物坐标决定（坐标变了颜色就变），既"随机"又稳定不闪烁
static Color FoodColorAt(int row, int col)
{
    return FOOD_PALETTE[(row * 7 + col) % 7];
}

// 画食物：七彩四角星芒（四个长芒 + 四个短谷，像闪光/星芒）
static void DrawFood(GameState* g)
{
    if (g->foodRow < 0 || g->foodCol < 0) return; // 棋盘被蛇占满等极端情况

    Rectangle r = CellRect(g->foodRow, g->foodCol);
    Vector2 center = { r.x + r.width / 2.0f, r.y + r.height / 2.0f };
    Vector2 pts[8];
    BuildStarPoints(center, CELL_SIZE * 0.48f, CELL_SIZE * 0.15f, pts); // 修长星芒：长芒 0.48 格、短谷 0.15 格

    Color color = FoodColorAt(g->foodRow, g->foodCol);

    // 用 8 个三角形拼成实心四角星芒（中心 + 相邻顶点）。
    // 顶点顺序必须是 pts[i+1] -> pts[i] -> center（绕向实测可见）；
    // 反过来会被渲染剔除，导致星形不显示（曾用 center -> pts[i] -> pts[i+1]
    // 和 DrawTriangleFan 两种画法，均不显示，对比测试确认本画法可靠）。
    for (int i = 0; i < 8; i++)
    {
        DrawTriangle(pts[(i + 1) % 8], pts[i], center, color);
    }
}

// ---------- 输入（物理按键轮询，不依赖窗口焦点）----------

// 按键"按下沿"检测：GetAsyncKeyState 读的是物理键盘状态，
// 即使窗口没聚焦也能收到，规避"按键被别的窗口抢走"的问题
static bool KeyJustPressed(int vk, bool* prevDown)
{
    bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool pressed = down && !*prevDown;
    *prevDown = down;
    return pressed;
}

// 方向键 + WASD 双通道（方向键优先，输入法不会拦截方向键）
static void HandleInput(GameState* g)
{
    static bool prevUp = false, prevDown = false, prevLeft = false, prevRight = false;
    static bool prevW = false, prevA = false, prevS = false, prevD = false;

    if (KeyJustPressed(VK_UP, &prevUp))    GameQueueDirection(g, DIR_UP);
    if (KeyJustPressed(VK_DOWN, &prevDown)) GameQueueDirection(g, DIR_DOWN);
    if (KeyJustPressed(VK_LEFT, &prevLeft)) GameQueueDirection(g, DIR_LEFT);
    if (KeyJustPressed(VK_RIGHT, &prevRight)) GameQueueDirection(g, DIR_RIGHT);

    if (KeyJustPressed('W', &prevW)) GameQueueDirection(g, DIR_UP);
    if (KeyJustPressed('S', &prevS)) GameQueueDirection(g, DIR_DOWN);
    if (KeyJustPressed('A', &prevA)) GameQueueDirection(g, DIR_LEFT);
    if (KeyJustPressed('D', &prevD)) GameQueueDirection(g, DIR_RIGHT);
}

// ---------- 暂停菜单（P5）----------

#define PAUSE_OPTION_COUNT 3

// 菜单底板与三个选项的屏幕矩形（绘制和鼠标点击共用同一组坐标）
static const Rectangle PAUSE_BOX = { 330.0f, 220.0f, 240.0f, 240.0f };
static const Rectangle PAUSE_OPTION_RECT[PAUSE_OPTION_COUNT] =
{
    { 370.0f, 286.0f, 160.0f, 38.0f },   // 继续
    { 370.0f, 336.0f, 160.0f, 38.0f },   // 重开
    { 370.0f, 386.0f, 160.0f, 38.0f }    // 退出
};

// 绘制暂停菜单：半透明遮罩 + 底板 + 标题 + 三个选项。
// *selectedIndex 是键盘选中的高亮项，鼠标悬停也会更新它。
// 返回鼠标左键点击的选项下标；本次没有点击返回 -1。
static int DrawPauseMenu(Font font, int* selectedIndex)
{
    // 1. 半透明遮罩盖住整个窗口，游戏画面冻结在下面
    DrawRectangleRec((Rectangle){ 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
                     (Color){ 0, 0, 0, 170 });

    // 2. 菜单底板
    DrawRectangleRec(PAUSE_BOX, (Color){ 12, 18, 46, 245 });
    DrawRectangleLinesEx(PAUSE_BOX, 2.0f, NEON_COLOR); // 霓虹描边

    // 3. 标题
    Vector2 titleSize = MeasureTextEx(font, PAUSE_TITLE_TEXT, 30.0f, 1.0f);
    DrawTextEx(font, PAUSE_TITLE_TEXT,
               (Vector2){ PAUSE_BOX.x + (PAUSE_BOX.width - titleSize.x) / 2.0f, PAUSE_BOX.y + 20.0f },
               30.0f, 1.0f, CAPTION_COLOR);

    // 4. 三个选项
    int clicked = -1;
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < PAUSE_OPTION_COUNT; i++)
    {
        Rectangle r = PAUSE_OPTION_RECT[i];
        bool hovered = CheckCollisionPointRec(mouse, r);
        bool selected = (i == *selectedIndex);
        if (hovered) *selectedIndex = i; // 鼠标悬停即选中（与键盘共用同一高亮）

        if (selected || hovered)
        {
            DrawRectangleRec(r, SNAKE_HEAD_COLOR);            // 高亮：亮黄底
        }
        else
        {
            DrawRectangleRec(r, (Color){ 42, 58, 108, 255 }); // 普通：深蓝底
        }
        if (selected || hovered)
        {
            DrawRectangleLinesEx(r, 2.0f, (Color){ 255, 255, 255, 210 }); // 选中项白描边
        }

        Color textColor = (selected || hovered) ? (Color){ 20, 24, 45, 255 } : HINT_COLOR;
        Vector2 textSize = MeasureTextEx(font, PAUSE_OPTION_TEXT[i], 22.0f, 1.0f);
        DrawTextEx(font, PAUSE_OPTION_TEXT[i],
                   (Vector2){ r.x + (r.width - textSize.x) / 2.0f, r.y + (r.height - textSize.y) / 2.0f - 1.0f },
                   22.0f, 1.0f, textColor);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clicked = i; // 鼠标点击选项
        }
    }
    return clicked;
}

// ---------- 屏幕状态机（P6：标题 → 游戏 → 暂停 → 死亡弹窗）----------

typedef enum ScreenState
{
    SCREEN_TITLE,   // 标题画面：点击/Enter 开始
    SCREEN_PLAYING, // 游戏中
    SCREEN_PAUSED,  // ESC 暂停菜单
    SCREEN_DEAD     // 死亡弹窗（分数 + 最高分 + 重开/退出）
} ScreenState;

// 重开一局：销毁旧局并重新初始化（蛇/食物/分数/速度/蛇身颜色全部复位）
static void RestartGame(GameState* g, float* tickAccumulator, Color* bodyColor)
{
    GameDestroy(g);
    GameInit(g, (unsigned int)time(NULL));
    *tickAccumulator = 0.0f;
    *bodyColor = SNAKE_BODY_COLOR;
}

// ---------- 右侧 HUD（分数 / 速度 / 档位 / 5 格加速进度条）----------
static void DrawHUD(Font font, GameState* g)
{
    // 右侧信息底板（半透明深色 + 霓虹描边，提升可读性）
    DrawRectangleRec((Rectangle){ 728.0f, 114.0f, 162.0f, 190.0f }, (Color){ 8, 12, 32, 190 });
    DrawRectangleLinesEx((Rectangle){ 728.0f, 114.0f, 162.0f, 190.0f }, 1.5f, (Color){ 53, 197, 240, 90 });

    float x = 738.0f;
    float y = 130.0f;
    char buf[64];

    // 分数
    snprintf(buf, sizeof(buf), "分数: %d", GameScore(g));
    DrawTextEx(font, buf, (Vector2){ x, y }, 20.0f, 1.0f, CAPTION_COLOR);
    y += 34.0f;

    // 速度（格/秒）
    snprintf(buf, sizeof(buf), "速度: %d 格/秒", GameSpeed(g));
    DrawTextEx(font, buf, (Vector2){ x, y }, 20.0f, 1.0f, HINT_COLOR);
    y += 34.0f;

    // 档位（每吃满 5 个升一档，0 = 初始档）
    snprintf(buf, sizeof(buf), "档位: %d", GameSpeedLevel(g));
    DrawTextEx(font, buf, (Vector2){ x, y }, 20.0f, 1.0f, HINT_COLOR);
    y += 46.0f;

    // 5 格加速进度条（已吃食物数 0..4，满 5 升档并归零）
    DrawTextEx(font, "加速进度", (Vector2){ x, y }, 18.0f, 1.0f, HINT_COLOR);
    y += 27.0f;
    int filled = GameFoodProgress(g);
    for (int i = 0; i < SNAKE_FOODS_PER_LEVEL; i++)
    {
        Rectangle cell = { x + i * 27.0f, y, 22.0f, 14.0f };
        Color c = (i < filled) ? SNAKE_BODY_COLOR : (Color){ 42, 58, 108, 255 };
        DrawRectangleRec(cell, c);
        DrawRectangleLinesEx(cell, 1.0f, (Color){ 90, 120, 200, 200 });
    }
}

// ---------- 标题画面 ----------
static void DrawTitleScreen(Font font, int highScore)
{
    // 不透明底色盖住下方游戏画面
    DrawRectangleRec((Rectangle){ 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT }, WINDOW_BG_COLOR);

    // 大标题
    Vector2 titleSize = MeasureTextEx(font, TITLE_TEXT, 76.0f, 1.0f);
    float titleLeft = (SCREEN_WIDTH - titleSize.x) / 2.0f;
    DrawTextEx(font, TITLE_TEXT,
               (Vector2){ titleLeft, 225.0f },
               76.0f, 1.0f, SNAKE_HEAD_COLOR);

    // 标题两侧的装饰星芒（复用可靠绕向的四角星画法）
    DrawSparkle((Vector2){ titleLeft - 62.0f, 262.0f }, 22.0f, NEON_COLOR);
    DrawSparkle((Vector2){ titleLeft + titleSize.x + 62.0f, 262.0f }, 22.0f, SNAKE_BODY_COLOR);

    // 最高分
    char buf[64];
    snprintf(buf, sizeof(buf), "最高分: %d", highScore);
    Vector2 hsSize = MeasureTextEx(font, buf, 24.0f, 1.0f);
    DrawTextEx(font, buf,
               (Vector2){ (SCREEN_WIDTH - hsSize.x) / 2.0f, 340.0f },
               24.0f, 1.0f, HINT_COLOR);

    // 开始提示
    Vector2 startSize = MeasureTextEx(font, TITLE_START_HINT, 22.0f, 1.0f);
    DrawTextEx(font, TITLE_START_HINT,
               (Vector2){ (SCREEN_WIDTH - startSize.x) / 2.0f, 420.0f },
               22.0f, 1.0f, CAPTION_COLOR);
}

// ---------- 死亡弹窗（分数 + 最高分 + 重开/退出）----------

#define DEATH_OPTION_COUNT 2

static const Rectangle DEATH_BOX = { 330.0f, 195.0f, 240.0f, 295.0f };
static const Rectangle DEATH_OPTION_RECT[DEATH_OPTION_COUNT] =
{
    { 370.0f, 400.0f, 160.0f, 38.0f },   // 重开
    { 370.0f, 446.0f, 160.0f, 38.0f }    // 退出
};

// 绘制死亡弹窗。isNewRecord：本局是否刷新了最高分。
// *selectedIndex 与暂停菜单一样由键盘/鼠标共同维护。
// 返回鼠标左键点击的选项下标；本次没有点击返回 -1。
static int DrawDeathPopup(Font font, GameState* g, int highScore, bool isNewRecord, int* selectedIndex)
{
    DrawRectangleRec((Rectangle){ 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
                     (Color){ 0, 0, 0, 170 });
    DrawRectangleRec(DEATH_BOX, (Color){ 12, 18, 46, 245 });
    DrawRectangleLinesEx(DEATH_BOX, 2.0f, NEON_COLOR); // 霓虹描边

    // 标题
    Vector2 titleSize = MeasureTextEx(font, DEATH_TITLE_TEXT, 30.0f, 1.0f);
    DrawTextEx(font, DEATH_TITLE_TEXT,
               (Vector2){ DEATH_BOX.x + (DEATH_BOX.width - titleSize.x) / 2.0f, DEATH_BOX.y + 16.0f },
               30.0f, 1.0f, DEAD_TEXT_COLOR);

    // 本局分数 / 最高分 / 新纪录提示
    char buf[128];
    snprintf(buf, sizeof(buf), "本局分数: %d", GameScore(g));
    DrawTextEx(font, buf, (Vector2){ DEATH_BOX.x + 24.0f, DEATH_BOX.y + 72.0f }, 22.0f, 1.0f, HINT_COLOR);
    snprintf(buf, sizeof(buf), "最高分: %d", highScore);
    DrawTextEx(font, buf, (Vector2){ DEATH_BOX.x + 24.0f, DEATH_BOX.y + 106.0f }, 22.0f, 1.0f, HINT_COLOR);
    if (isNewRecord)
    {
        DrawTextEx(font, NEW_RECORD_TEXT,
                   (Vector2){ DEATH_BOX.x + 24.0f, DEATH_BOX.y + 142.0f },
                   20.0f, 1.0f, SNAKE_HEAD_COLOR);
    }

    // 重开 / 退出 两个选项
    int clicked = -1;
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < DEATH_OPTION_COUNT; i++)
    {
        Rectangle r = DEATH_OPTION_RECT[i];
        bool hovered = CheckCollisionPointRec(mouse, r);
        bool selected = (i == *selectedIndex);
        if (hovered) *selectedIndex = i;

        if (selected || hovered)
        {
            DrawRectangleRec(r, SNAKE_HEAD_COLOR);
        }
        else
        {
            DrawRectangleRec(r, (Color){ 42, 58, 108, 255 });
        }
        if (selected || hovered)
        {
            DrawRectangleLinesEx(r, 2.0f, (Color){ 255, 255, 255, 210 });
        }

        Color textColor = (selected || hovered) ? (Color){ 20, 24, 45, 255 } : HINT_COLOR;
        Vector2 textSize = MeasureTextEx(font, DEATH_OPTION_TEXT[i], 22.0f, 1.0f);
        DrawTextEx(font, DEATH_OPTION_TEXT[i],
                   (Vector2){ r.x + (r.width - textSize.x) / 2.0f, r.y + (r.height - textSize.y) / 2.0f - 1.0f },
                   22.0f, 1.0f, textColor);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            clicked = i;
        }
    }
    return clicked;
}

int main(void)
{
    // 1. 创建窗口：标题使用 UTF-8 中文
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "贪吃蛇");
    SetExitKey(0); // 禁用 raylib 默认的"ESC 关闭窗口"快捷键，ESC 只用于暂停/继续
    SetWindowFocused(); // 主动请求键盘焦点（能否成功受 Windows 前台锁限制）
    SetTargetFPS(60);

    // 2. 加载中文字体；加载失败则退回 raylib 默认字体（中文无法显示，仅兜底）
    Font font = LoadChineseFont(30);
    if (font.texture.id == 0)
    {
        font = GetFontDefault();
    }

    // 3. 音效与最高分存档
    InitAudioDevice(); // 初始化音频设备（吃食物/死亡音效）
    Sound eatSfx = LoadSound("resources/eat.wav");
    Sound deathSfx = LoadSound("resources/death.wav");
    char highScorePath[512];
    snprintf(highScorePath, sizeof(highScorePath), "%shighscore.txt", GetApplicationDirectory());
    int highScore = HighScoreLoad(highScorePath);

    // 4. 初始化一局：以当前时间做随机种子，食物位置每次启动都不同
    GameState g;
    GameInit(&g, (unsigned int)time(NULL));

    float tickAccumulator = 0.0f;   // 累计帧时间，到点推进一格
    static bool prevEsc = false;    // ESC 也用物理轮询，保证能收到
    static bool prevMenuUp = false, prevMenuDown = false, prevMenuEnter = false; // 暂停菜单按键沿
    static bool prevDeathUp = false, prevDeathDown = false, prevDeathEnter = false; // 死亡弹窗按键沿
    static bool prevStartEnter = false, prevStartSpace = false; // 标题开始按键沿
    ScreenState screen = SCREEN_TITLE; // 初始进入标题画面
    int pauseMenuIndex = 0;         // 暂停菜单选中项（0=继续 1=重开 2=退出）
    int deathMenuIndex = 0;         // 死亡弹窗选中项（0=重开 1=退出）
    Color snakeBodyColor = SNAKE_BODY_COLOR; // 蛇身当前颜色：初始霓虹绿，吃到什么颜色食物就变什么颜色
    bool isNewRecord = false;       // 本局是否刷新最高分（死亡结算时判定）
    bool quit = false;              // 退出游戏标志（switch 内不能直接 break 主循环）

    // 5. 主循环
    while (!WindowShouldClose() && !quit)
    {
        // --- 输入与状态切换 ---
        switch (screen)
        {
        case SCREEN_TITLE:
            // 标题画面：点击鼠标 或 按 Enter/Space 开始新一局
            if (KeyJustPressed(VK_RETURN, &prevStartEnter) ||
                KeyJustPressed(' ', &prevStartSpace) ||
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                RestartGame(&g, &tickAccumulator, &snakeBodyColor);
                screen = SCREEN_PLAYING;
            }
            break;

        case SCREEN_PLAYING:
            // 游戏中：存活时 ESC 暂停；转向输入照常
            if (GameIsAlive(&g) && KeyJustPressed(VK_ESCAPE, &prevEsc))
            {
                screen = SCREEN_PAUSED;
                pauseMenuIndex = 0;
            }
            HandleInput(&g);
            break;

        case SCREEN_PAUSED:
            // 暂停菜单：ESC=继续；↑/↓=选择；Enter=确认（鼠标点击在渲染阶段处理）
            if (KeyJustPressed(VK_ESCAPE, &prevEsc))
            {
                screen = SCREEN_PLAYING; // 继续
            }
            else if (KeyJustPressed(VK_UP, &prevMenuUp))
            {
                pauseMenuIndex = (pauseMenuIndex + PAUSE_OPTION_COUNT - 1) % PAUSE_OPTION_COUNT;
            }
            else if (KeyJustPressed(VK_DOWN, &prevMenuDown))
            {
                pauseMenuIndex = (pauseMenuIndex + 1) % PAUSE_OPTION_COUNT;
            }
            else if (KeyJustPressed(VK_RETURN, &prevMenuEnter))
            {
                if (pauseMenuIndex == 0)
                {
                    screen = SCREEN_PLAYING; // 继续
                }
                else if (pauseMenuIndex == 1)
                {
                    RestartGame(&g, &tickAccumulator, &snakeBodyColor); // 重开
                    screen = SCREEN_PLAYING;
                }
                else
                {
                    quit = true; // 退出游戏
                }
            }
            break;

        case SCREEN_DEAD:
            // 死亡弹窗：↑/↓=切换重开/退出；Enter=确认（鼠标点击在渲染阶段处理）
            if (KeyJustPressed(VK_UP, &prevDeathUp) || KeyJustPressed(VK_DOWN, &prevDeathDown))
            {
                deathMenuIndex = 1 - deathMenuIndex;
            }
            else if (KeyJustPressed(VK_RETURN, &prevDeathEnter))
            {
                if (deathMenuIndex == 0)
                {
                    RestartGame(&g, &tickAccumulator, &snakeBodyColor); // 重开
                    screen = SCREEN_PLAYING;
                }
                else
                {
                    quit = true; // 退出游戏
                }
            }
            break;
        }

        // --- 按速度驱动 tick：每 1/速度 秒前进一格（仅游戏中且存活）---
        if (screen == SCREEN_PLAYING && GameIsAlive(&g))
        {
            tickAccumulator += GetFrameTime();
            float interval = 1.0f / (float)GameSpeed(&g);
            while (tickAccumulator >= interval)
            {
                // 记录吃食物前的分数/食物坐标、存活状态，用于变色与音效
                int scoreBeforeTick = g.score;
                int oldFoodRow = g.foodRow;
                int oldFoodCol = g.foodCol;
                int aliveBeforeTick = GameIsAlive(&g);

                GameTick(&g);

                if (g.score > scoreBeforeTick)
                {
                    // 吃到食物：蛇身颜色 = 刚吃掉的那颗食物的颜色（蛇头始终亮黄不变）+ 音效
                    snakeBodyColor = FoodColorAt(oldFoodRow, oldFoodCol);
                    PlaySound(eatSfx);
                }
                if (aliveBeforeTick && !GameIsAlive(&g))
                {
                    PlaySound(deathSfx); // 本 tick 撞死
                }

                tickAccumulator -= interval;
                if (!GameIsAlive(&g)) break; // 刚撞死就停止推进
            }
        }

        // --- 死亡进入结算弹窗（只进一次；刷新最高分则写存档）---
        if (screen == SCREEN_PLAYING && !GameIsAlive(&g))
        {
            screen = SCREEN_DEAD;
            deathMenuIndex = 0;
            isNewRecord = (g.score > highScore);
            if (isNewRecord)
            {
                highScore = g.score;
                HighScoreSave(highScorePath, highScore);
            }
        }

        // --- 渲染 ---
        BeginDrawing();
        ClearBackground(WINDOW_BG_COLOR);

        // 标题画面之外都绘制游戏场景
        if (screen != SCREEN_TITLE)
        {
            // 顶部说明文字
            Vector2 capSize = MeasureTextEx(font, CAPTION_TEXT, 30.0f, 1.0f);
            DrawTextEx(font, CAPTION_TEXT,
                       (Vector2){ (SCREEN_WIDTH - capSize.x) / 2.0f, 42.0f },
                       30.0f, 1.0f, CAPTION_COLOR);

            DrawBoard();                     // 棋盘
            DrawFood(&g);                    // 食物（画在蛇下面，避免遮住蛇）
            DrawSnake(&g, snakeBodyColor);   // 蛇（蛇身随吃到的食物变色，蛇头不变）
            DrawHUD(font, &g);               // 右侧 HUD：分数/速度/档位/进度条

            // 底部提示文字
            Vector2 hintSize = MeasureTextEx(font, HINT_TEXT, 18.0f, 1.0f);
            DrawTextEx(font, HINT_TEXT,
                       (Vector2){ (SCREEN_WIDTH - hintSize.x) / 2.0f, 665.0f },
                       18.0f, 1.0f, HINT_COLOR);
        }

        // 覆盖层：标题 / 暂停 / 死亡弹窗（鼠标点击在绘制时检测，点中后在此处理）
        if (screen == SCREEN_TITLE)
        {
            DrawTitleScreen(font, highScore);
        }
        else if (screen == SCREEN_PAUSED)
        {
            int clicked = DrawPauseMenu(font, &pauseMenuIndex);
            if (clicked >= 0)
            {
                if (clicked == 0)
                {
                    screen = SCREEN_PLAYING; // 继续
                }
                else if (clicked == 1)
                {
                    RestartGame(&g, &tickAccumulator, &snakeBodyColor); // 重开
                    screen = SCREEN_PLAYING;
                }
                else
                {
                    quit = true; // 退出游戏
                }
            }
        }
        else if (screen == SCREEN_DEAD)
        {
            int clicked = DrawDeathPopup(font, &g, highScore, isNewRecord, &deathMenuIndex);
            if (clicked >= 0)
            {
                if (clicked == 0)
                {
                    RestartGame(&g, &tickAccumulator, &snakeBodyColor); // 重开
                    screen = SCREEN_PLAYING;
                }
                else
                {
                    quit = true; // 退出游戏
                }
            }
        }

        EndDrawing();
    }

    // 6. 收尾：释放音效/游戏/字体，关闭窗口
    GameDestroy(&g);
    UnloadSound(eatSfx);
    UnloadSound(deathSfx);
    CloseAudioDevice();
    UnloadFont(font);
    CloseWindow();
    return 0;
}

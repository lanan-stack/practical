// ============================================================
// ui_screens.c —— 各画面的绘制实现
// ------------------------------------------------------------
// 所属层次：界面层
//
// 统一风格：标题类文字用霓虹绿，说明性文字用柔和的浅蓝灰，
// 选中项用霓虹绿、未选中项用浅蓝灰 —— 与棋盘、面板保持同一套视觉语言。
// ============================================================
#include "ui_screens.h"
#include "ui_config.h"
#include "ui_text.h"

#include <stdio.h>
#include <math.h>

// ============================================================
// 内部辅助：把一行字水平居中画在指定高度
// ============================================================
static void DrawCentered(const Font* font, const char* text, float y, float size, Color color)
{
    Vector2 measured = MeasureTextEx(*font, text, size, 1.0f);
    DrawTextEx(*font, text,
               (Vector2){ ((float)WINDOW_WIDTH - measured.x) / 2.0f, y },
               size, 1.0f, color);
}

// ============================================================
// 内部辅助：发光的文字
// ------------------------------------------------------------
// 做法：先用几层低透明度的同色文字往外叠，再画一层实心的正文，
// 看起来就像霓虹灯管在发光。层数不多，开销可以忽略。
// ============================================================
static void DrawCenteredGlow(const Font* font, const char* text, float y, float size,
                             Color color, float glow)
{
    for (int i = 6; i >= 1; i--)
    {
        Color halo = color;
        halo.a = (unsigned char)(glow * 255.0f / (float)i);

        Vector2 measured = MeasureTextEx(*font, text, size, 1.0f);
        const float x = ((float)WINDOW_WIDTH - measured.x) / 2.0f;

        DrawTextEx(*font, text, (Vector2){ x, y }, size, 1.0f, halo);
        DrawTextEx(*font, text, (Vector2){ x + i * 0.7f, y + i * 0.7f }, size, 1.0f, halo);
    }

    Vector2 measured = MeasureTextEx(*font, text, size, 1.0f);
    DrawTextEx(*font, text, (Vector2){ ((float)WINDOW_WIDTH - measured.x) / 2.0f, y },
               size, 1.0f, color);
}

// ============================================================
// 内部辅助：画一个很淡的装饰方块
// ------------------------------------------------------------
// 与棋盘上的方块同一种画法（暗底 + 本色主体 + 左上高光），只是整体透明度很低，
// 用来当背景装饰 —— 看得见它有方块的味道，又不会抢走文字的注意力。
// ============================================================
static void DrawFaintCell(float x, float y, float size, Color color, float alpha)
{
    Color body  = Fade(color, alpha);
    Color edge  = Fade(ColorBrightness(color, -0.55f), alpha);
    Color shine = Fade(ColorBrightness(color, 0.35f), alpha);

    DrawRectangle((int)x, (int)y, (int)size, (int)size, edge);
    DrawRectangle((int)x + 2, (int)y + 2, (int)size - 4, (int)size - 4, body);
    DrawRectangle((int)x + 2, (int)y + 2, (int)size - 4, 2, shine);
    DrawRectangle((int)x + 2, (int)y + 2, 2, (int)size - 4, shine);
}

// ============================================================
// 内部辅助：方块背景墙（方案 B）
// ------------------------------------------------------------
// 整屏铺一层很淡的方块，**越靠屏幕边缘越明显、越靠中心越淡**，
// 自然形成"中间聚光"的效果，为卡片和文字让出干净的背景。
// 颜色按格子位置轮换七种方块色，所以远看是一片有层次的暗色网格。
// ============================================================
static void DrawBlockWall(void)
{
    const float size = 48.0f;
    const int cols = WINDOW_WIDTH / (int)size + 1;
    const int rows = WINDOW_HEIGHT / (int)size + 1;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            const float cx = (float)c * size + size / 2.0f;
            const float cy = (float)r * size + size / 2.0f;

            // 到屏幕中心的归一化距离（0 = 正中，1 = 边缘）
            const float dx = (cx - WINDOW_WIDTH / 2.0f) / (WINDOW_WIDTH / 2.0f);
            const float dy = (cy - WINDOW_HEIGHT / 2.0f) / (WINDOW_HEIGHT / 2.0f);
            const float dist = sqrtf(dx * dx + dy * dy);

            const float alpha = 0.04f + 0.08f * dist * dist;
            const int colorIndex = (r * 3 + c * 5) % TETROMINO_COUNT;

            DrawFaintCell((float)c * size, (float)r * size, size,
                          BLOCK_COLORS[colorIndex], alpha);
        }
    }
}

// ============================================================
// 内部辅助：背景里缓慢下落的方块（动效之一）
// ------------------------------------------------------------
// 六块方块各自从屏幕上方落下、穿过去之后在底部淡出，再从上方淡入，如此循环。
// 设计要点：
//   · 速度很慢（每秒 55~90 像素，走完全程要 17~27 秒），只是让画面"活着"，
//     不会让人分神；
//   · 透明度很低（最高 0.16），并且进出屏幕各留 18% 的行程用来淡入淡出，
//     所以不会看到"凭空出现 / 突然消失"；
//   · 每块的形状、旋转、横坐标、相位都不同，避免看起来像一条整齐的传送带。
// ============================================================
typedef struct FallingPiece
{
    int   id;      // 方块编号
    int   rotate;  // 旋转几次（0~3），让六块看起来更自然
    float x;       // 横坐标
    float phase;   // 相位偏移：让它们错开，不会同时出现
    float size;    // 每格边长
} FallingPiece;

static const FallingPiece FALLING[6] =
{
    { TETROMINO_T, 0,  70.0f,   0.0f, 46.0f },
    { TETROMINO_L, 1, 180.0f, 220.0f, 40.0f },
    { TETROMINO_I, 1, 300.0f, 480.0f, 34.0f },
    { TETROMINO_S, 0, 470.0f,  90.0f, 44.0f },
    { TETROMINO_Z, 2, 600.0f, 360.0f, 42.0f },
    { TETROMINO_O, 0, 690.0f, 640.0f, 38.0f }
};

static void DrawFallingPieces(double time)
{
    const float cycle = 1500.0f;   // 一趟的总行程（含屏幕上下方预留的余量）
    const float margin = 260.0f;   // 屏幕上方预留的距离

    for (int i = 0; i < 6; i++)
    {
        const float speed = 55.0f + i * 7.0f;   // 像素/秒
        const float y = (float)fmod(time * speed + FALLING[i].phase, cycle) - margin;

        // 进出屏幕时淡入淡出
        const float t = (y + margin) / cycle;
        float alpha = 0.16f;
        if (t < 0.18f) { alpha *= t / 0.18f; }
        if (t > 0.82f) { alpha *= (1.0f - t) / 0.18f; }
        if (alpha <= 0.002f) { continue; }

        int box[TETROMINO_BOX][TETROMINO_BOX];
        if (!tetromino_base_copy(FALLING[i].id, box)) { continue; }
        for (int k = 0; k < FALLING[i].rotate; k++)
        {
            tetromino_rotate_cw(box);
        }

        const float size = FALLING[i].size;
        const Color color = BLOCK_COLORS[FALLING[i].id];

        for (int r = 0; r < TETROMINO_BOX; r++)
        {
            for (int c = 0; c < TETROMINO_BOX; c++)
            {
                if (box[r][c])
                {
                    DrawFaintCell(FALLING[i].x + c * size, y + r * size, size, color, alpha);
                }
            }
        }
    }
}

// ============================================================
// 内部辅助：中间的卡片（方案 C）
// ------------------------------------------------------------
// 半透明深蓝底 + 霓虹边框 + 四个角的小装饰。
// 四角装饰是为了让边框不至于是个死板的方框，多一分精致。
// ============================================================
static void DrawInfoCard(Rectangle card)
{
    DrawRectangleRec(card, Fade(COLOR_BOARD_BG, 0.88f));
    DrawRectangleLinesEx(card, 2.0f, Fade(COLOR_NEON, 0.75f));

    const float k = 18.0f;   // 每个角的小装饰边长
    DrawRectangleLinesEx((Rectangle){ card.x, card.y, k, k }, 3.0f, COLOR_NEON);
    DrawRectangleLinesEx((Rectangle){ card.x + card.width - k, card.y, k, k }, 3.0f, COLOR_NEON);
    DrawRectangleLinesEx((Rectangle){ card.x, card.y + card.height - k, k, k }, 3.0f, COLOR_NEON);
    DrawRectangleLinesEx((Rectangle){ card.x + card.width - k,
                                      card.y + card.height - k, k, k }, 3.0f, COLOR_NEON);
}

// ============================================================
// 标题画面（方案 B + C 组合）
// ------------------------------------------------------------
// 背景是方块墙，中间一块卡片装着全部信息：
//   游戏名（带呼吸式发光）→ 分隔线 → 三档最高分 → 呼吸式开始提示。
// 背景是两层的：静态的方块墙（越靠边越明显）+ 六块缓慢下落的方块（动效）。
// ============================================================
void ScreenDrawTitle(const Font* font, const HighScores* scores, double time)
{
    // ---------- 背景 ----------
    DrawBlockWall();
    DrawFallingPieces(time);

    // ---------- 卡片 ----------
    const Rectangle card = { 90.0f, 200.0f, (float)WINDOW_WIDTH - 180.0f, 560.0f };
    DrawInfoCard(card);

    // ---------- 游戏名（外发光随呼吸轻微明暗变化）----------
    // 在 0.30 ~ 0.60 之间缓慢来回，周期约 3 秒 —— 有生命力，但不刺眼。
    const float glowPulse = (float)(0.45 + 0.15 * sin(time * 2.0));
    DrawCenteredGlow(font, TEXT_TITLE, 250.0f, 72.0f, COLOR_CAPTION, glowPulse);

    // ---------- 分隔线：中间亮、两端淡出 ----------
    const int lineWidth = 240;
    for (int i = 0; i < lineWidth; i++)
    {
        const float t = (float)i / (float)(lineWidth - 1);
        const float fade = 1.0f - fabsf(t - 0.5f) * 2.0f;
        Color c = COLOR_NEON;
        c.a = (unsigned char)(fade * 200.0f);
        DrawRectangle((WINDOW_WIDTH - lineWidth) / 2 + i, 352, 1, 2, c);
    }

    // ---------- 三档最高分 ----------
    DrawCentered(font, TEXT_BEST_LIST, 386.0f, 24.0f, COLOR_HINT);

    const char* names[DIFFICULTY_COUNT] = { TEXT_DIFF_EASY, TEXT_DIFF_NORMAL, TEXT_DIFF_HARD };
    for (int i = 0; i < DIFFICULTY_COUNT; i++)
    {
        char line[64];
        sprintf_s(line, sizeof(line), "%s    %d", names[i], scores->best[i]);
        DrawCentered(font, line, 430.0f + i * 48.0f, 30.0f, COLOR_NEON);
    }

    // "按 Enter 开始"：用正弦做呼吸式明暗变化 —— 有存在感，但不刺眼、不闪跳。
    const double breath = 0.5 + 0.5 * sin(time * 3.0);
    Color hintColor = COLOR_HINT;
    hintColor.a = (unsigned char)(110 + 130 * breath);
    DrawCentered(font, TEXT_START_HINT, 640.0f, 32.0f, hintColor);
}

// ============================================================
// 难度选择
// ============================================================
void ScreenDrawDifficulty(const Font* font, int selected, double time)
{
    DrawCentered(font, TEXT_DIFFICULTY_TITLE, 110.0f, 46.0f, COLOR_CAPTION);

    const char* names[DIFFICULTY_COUNT] = { TEXT_DIFF_EASY, TEXT_DIFF_NORMAL, TEXT_DIFF_HARD };
    // 三档的封顶速度（与 game_logic 的速度表一致），直接写成人可读的文本
    const char* capText[DIFFICULTY_COUNT] = { "0.36", "0.13", "0.043" };

    for (int i = 0; i < DIFFICULTY_COUNT; i++)
    {
        const bool isSelected = (i == selected);

        // 动效之三：选中项轻微上浮并呼吸（上浮 0~6 像素），一眼就能看出焦点在哪。
        const float lift = isSelected ? (float)(3.0 + 3.0 * sin(time * 3.0)) : 0.0f;
        const float top = 210.0f + i * 150.0f - lift;

        if (isSelected)
        {
            Rectangle frame = { PANEL_X - 150.0f, top - 16.0f, 460.0f, 110.0f };
            DrawRectangleLinesEx(frame, 2.0f, COLOR_NEON);
        }

        DrawCentered(font, names[i], top, 40.0f, isSelected ? COLOR_CAPTION : COLOR_HINT);

        char speedLine[64];
        sprintf_s(speedLine, sizeof(speedLine), TEXT_DIFF_SPEED_FORMAT, capText[i]);
        DrawCentered(font, speedLine, top + 52.0f, 22.0f, COLOR_HINT);
    }

    DrawCentered(font, TEXT_MENU_HINT, 720.0f, 22.0f, COLOR_HINT);
}

// ============================================================
// 暂停菜单
// ============================================================
void ScreenDrawPauseOverlay(const Font* font, int selected)
{
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.62f));
    DrawCentered(font, TEXT_PAUSED, 300.0f, 50.0f, COLOR_CAPTION);

    const char* items[3] = { TEXT_MENU_RESUME, TEXT_MENU_RESTART, TEXT_MENU_BACK };
    for (int i = 0; i < 3; i++)
    {
        DrawCentered(font, items[i], 430.0f + i * 72.0f, 34.0f,
                     (i == selected) ? COLOR_CAPTION : COLOR_HINT);
    }

    DrawCentered(font, TEXT_MENU_HINT, 720.0f, 22.0f, COLOR_HINT);
}

// ============================================================
// 结算画面
// ============================================================
void ScreenDrawGameOverOverlay(const Font* font, const GameState* game,
                               const HighScores* scores, bool newRecord, int selected)
{
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.70f));
    DrawCentered(font, TEXT_GAME_OVER, 150.0f, 56.0f, COLOR_DEAD);
    DrawCentered(font, TEXT_RESULT_TITLE, 225.0f, 26.0f, COLOR_HINT);

    char line[96];

    sprintf_s(line, sizeof(line), "%s    %d", TEXT_RESULT_SCORE, game->score);
    DrawCentered(font, line, 280.0f, 30.0f, COLOR_NEON);

    sprintf_s(line, sizeof(line), "%s    %d", TEXT_RESULT_LINES, game->totalLines);
    DrawCentered(font, line, 322.0f, 30.0f, COLOR_NEON);

    sprintf_s(line, sizeof(line), "%s    %d", TEXT_RESULT_LEVEL, game->level);
    DrawCentered(font, line, 364.0f, 30.0f, COLOR_NEON);

    sprintf_s(line, sizeof(line), "%s    %d", TEXT_RESULT_BEST,
              scores->best[game->difficulty]);
    DrawCentered(font, line, 418.0f, 28.0f, COLOR_HINT);

    if (newRecord)
    {
        DrawCentered(font, TEXT_NEW_RECORD, 470.0f, 38.0f, COLOR_CAPTION);
    }

    const char* items[2] = { TEXT_MENU_RESTART, TEXT_MENU_BACK };
    for (int i = 0; i < 2; i++)
    {
        DrawCentered(font, items[i], 570.0f + i * 72.0f, 34.0f,
                     (i == selected) ? COLOR_CAPTION : COLOR_HINT);
    }

    DrawCentered(font, TEXT_MENU_HINT, 730.0f, 22.0f, COLOR_HINT);
}

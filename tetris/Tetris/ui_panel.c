// ============================================================
// ui_panel.c —— 右侧信息栏绘制的实现
// ------------------------------------------------------------
// 所属层次：界面层
//
// 面板自上而下分三个框：HOLD（暂存）、NEXT（预览 3 个）、统计信息。
// 每个框的画法都是"描边矩形 + 左上角标签 + 框内内容"，风格统一。
// ============================================================
#include "ui_panel.h"
#include "ui_config.h"
#include "ui_text.h"
#include "tetromino.h"   // 真实形状数据（暂存区与预览区按形状编号取用）
#include <stdio.h>   // 只用于把数字格式化成字符串（TextFormat 之外的保险做法）

// ============================================================
// 内部辅助函数：画一个带标签的框
// ------------------------------------------------------------
// 参数：x, y, w, h —— 框的位置与大小
//       label    —— 框左上角的标签文字（中文，如 暂存 / 预览）
//       font     —— 字体
// 返回：无
// 说明：所有面板框都走这一个函数，保证边框颜色、粗细、内边距完全一致。
// ============================================================
static void DrawLabeledBox(int x, int y, int w, int h, const char* label, const Font* font)
{
    // 框体：细描边，不填充（让深色背景透出来，保持干净的观感）
    Rectangle box = { (float)x, (float)y, (float)w, (float)h };
    DrawRectangleLinesEx(box, 1.5f, COLOR_PANEL_LINE);

    // 标签：放在框内左上角，霓虹青
    Vector2 labelPos = { (float)(x + 14), (float)(y + 10) };
    DrawTextEx(*font, label, labelPos, PANEL_LABEL_SIZE, 1.0f, COLOR_NEON);
}

// ============================================================
// 内部辅助函数：在指定中心点画一个"迷你方块"
// ------------------------------------------------------------
// 参数：cx, cy —— 方块整体的中心点（像素）
//       cell   —— 迷你方块每格边长
//       color  —— 方块颜色
//       box    —— 形状数据（4×4 盒子，1 表示有方块），由 tetromino 模块提供
// 返回：无
//
// 【画法要点：经典俄罗斯方块的"小方块拼装"效果】
// 第一步先算出"有方块的范围"并整体居中，所以不同形状都会摆正中间，不会东倒西歪。
// 第二步逐格绘制，每一格分三层画，让"四个小方块"看得清清楚楚：
//   1) 底层：把本色压暗当分隔线，相邻格子之间自然形成清晰的分隔；
//   2) 主体：四周内缩 2 像素，露出分隔线，形成小方格轮廓；
//   3) 高光：左上角描两条亮边，模拟经典方块的光泽。
// 这样黄色 O 形看起来就是"四个小黄方块拼成的方块"，
// 而不是一个糊在一起的大色块 —— 这正是经典俄罗斯方块的观感。
// ============================================================
static void DrawMiniTetrominoCentered(float cx, float cy, float cell, Color color,
                                      const int box[TETROMINO_BOX][TETROMINO_BOX])
{
    // ---------- 1. 求"有方块"的包围盒 ----------
    int minCol = TETROMINO_BOX, maxCol = -1;
    int minRow = TETROMINO_BOX, maxRow = -1;

    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            if (box[r][c])
            {
                if (c < minCol) { minCol = c; }
                if (c > maxCol) { maxCol = c; }
                if (r < minRow) { minRow = r; }
                if (r > maxRow) { maxRow = r; }
            }
        }
    }
    if (maxCol < 0)
    {
        return;   // 盒子里一个方块都没有（正常不会发生），直接不画
    }

    // ---------- 2. 算出整体尺寸，进而算出左上角起点 ----------
    float width  = (float)(maxCol - minCol + 1) * cell;
    float height = (float)(maxRow - minRow + 1) * cell;
    float x0 = cx - width / 2.0f;
    float y0 = cy - height / 2.0f;

    // ---------- 3. 逐格绘制：每个小方块三层画法 ----------
    // 颜色由三个色阶构成：暗色描边 + 本色主体 + 亮色高光。
    Color edge    = ColorBrightness(color, -0.55f);  // 压暗，用作各小格之间的分隔
    Color body    = color;                           // 本色主体
    Color shine   = ColorBrightness(color,  0.35f);  // 左上角高光

    const int inset  = 2;                            // 主体相对格子边缘内缩的像素
    const int border = 2;                            // 高光细边的粗细

    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            if (!box[r][c])
            {
                continue;   // 这一格是空的
            }

            int x = (int)(x0 + (float)(c - minCol) * cell);
            int y = (int)(y0 + (float)(r - minRow) * cell);
            int size = (int)cell;

            // 第 1 层：暗色底 —— 相邻两格的暗底拼在一起就成了一条分隔线
            DrawRectangle(x, y, size, size, edge);

            // 第 2 层：本色主体（四周内缩，露出暗底形成"小方格"轮廓）
            DrawRectangle(x + inset, y + inset, size - inset * 2, size - inset * 2, body);

            // 第 3 层：左上两条亮边，做出立体的光泽感
            DrawRectangle(x + inset, y + inset, size - inset * 2, border, shine);      // 顶边
            DrawRectangle(x + inset, y + inset, border, size - inset * 2, shine);      // 左边
        }
    }
}

// ============================================================
// 画 HOLD（暂存区）
// ------------------------------------------------------------
// Phase 5：画真实暂存的方块。槽里没有方块时只画空框，
//          这样玩家一眼就能看出"我还没暂存过任何东西"。
// ============================================================
void PanelDrawHoldBox(const Font* font, const Hold* hold)
{
    DrawLabeledBox(PANEL_X, HOLD_BOX_Y, PANEL_W, HOLD_BOX_H, TEXT_HOLD, font);

    if (hold == NULL || !hold->hasPiece)
    {
        return;   // 槽是空的：只留一个空框
    }

    // 取槽里那块方块的形状数据
    int box[TETROMINO_BOX][TETROMINO_BOX];
    if (!tetromino_base_copy(hold->slotId, box))
    {
        return;
    }

    // 居中点：框的水平中点，纵向在标签下方居中
    float cx = (float)(PANEL_X + PANEL_W / 2);
    float cy = (float)(HOLD_BOX_Y + HOLD_BOX_H / 2 + 12);
    DrawMiniTetrominoCentered(cx, cy, 26.0f, BlockColorOf(hold->slotId), box);
}

// ============================================================
// 画 NEXT（预览区，固定 3 个）
// ------------------------------------------------------------
// Phase 5：画袋子系统给出的真实预览（接下来 3 个方块）。
//          调用方每帧把最新预览取好传进来，所以补袋之后画面会自动往前滚。
// ============================================================
void PanelDrawNextBox(const Font* font, const int nextIds[BAG_PREVIEW_COUNT])
{
    DrawLabeledBox(PANEL_X, NEXT_BOX_Y, PANEL_W, NEXT_BOX_H, TEXT_NEXT, font);

    // 三个格子等分整个 NEXT 框的高度
    float slotH = (float)NEXT_BOX_H / (float)NEXT_SLOT_COUNT;

    for (int i = 0; i < NEXT_SLOT_COUNT; i++)
    {
        const int id = nextIds[i];

        int box[TETROMINO_BOX][TETROMINO_BOX];
        if (!tetromino_base_copy(id, box))
        {
            continue;   // 编号非法（正常不会发生）
        }

        // 中心点 y：第 i 个格子的垂直中点，再往下偏 10 像素给标签让位
        float cy = (float)NEXT_BOX_Y + slotH * ((float)i + 0.5f) + 10.0f;
        float cx = (float)(PANEL_X + PANEL_W / 2);

        // 颜色按形状编号取，与棋盘上的方块保持一致
        DrawMiniTetrominoCentered(cx, cy, 26.0f, BlockColorOf(id), box);
    }
}

// ============================================================
// 画统计框（分数 / 等级 / 行数 / 连击）
// ------------------------------------------------------------
// 排版规则：左边是中文标签（分数 / 等级 / 行数），右边是右对齐的数值。
// 连击提示放在框的右下角，字号刻意小一号、颜色偏暗 —— 这就是规格里要求的
// "安静地显示在角落，不打断视线"。
// ============================================================
void PanelDrawStatsBox(const Font* font, int score, int level, int lines, int combo)
{
    DrawLabeledBox(PANEL_X, STATS_BOX_Y, PANEL_W, STATS_BOX_H, TEXT_STATS, font);

    // 标签文字与数值：三行等距排列
    const char* labels[3] = { TEXT_SCORE, TEXT_LEVEL, TEXT_LINES };
    int         values[3] = { score, level, lines };

    for (int i = 0; i < 3; i++)
    {
        float rowY = (float)(STATS_BOX_Y + 46 + i * 38);

        // 左侧标签
        DrawTextEx(*font, labels[i], (Vector2){ (float)(PANEL_X + 14), rowY },
                   PANEL_LABEL_SIZE, 1.0f, COLOR_HINT);

        // 右侧数值：先量出文字宽度，再让它右对齐到框内侧留出 14 像素的位置
        char buf[32];
        sprintf_s(buf, sizeof(buf), "%d", values[i]);
        Vector2 size = MeasureTextEx(*font, buf, (float)PANEL_VALUE_SIZE, 1.0f);
        float x = (float)(PANEL_X + PANEL_W - 14) - size.x;
        DrawTextEx(*font, buf, (Vector2){ x, rowY - 2.0f },
                   PANEL_VALUE_SIZE, 1.0f, COLOR_CAPTION);
    }

    // ---------- 连击提示（只在连击 ≥ 2 时出现）----------
    // 注意：Phase 1 是静态画面，用写死的数值 2 演示位置和样式。
    if (combo >= 2)
    {
        char buf[32];
        sprintf_s(buf, sizeof(buf), TEXT_COMBO_FORMAT, combo);
        Vector2 size = MeasureTextEx(*font, buf, (float)COMBO_FONT_SIZE, 1.0f);
        float x = (float)(PANEL_X + PANEL_W - 14) - size.x;
        float y = (float)(STATS_BOX_Y + STATS_BOX_H - 26);

        // 颜色取半透明的提示色：看得见，但不打扰
        DrawTextEx(*font, buf, (Vector2){ x, y }, (float)COMBO_FONT_SIZE, 1.0f,
                   Fade(COLOR_HINT, 0.75f));
    }
}

// ============================================================
// 一次性画完整个信息栏
// ------------------------------------------------------------
// 数据来源：暂存区与预览区都取自游戏状态（真实数据）；
//           统计区在 Phase 5 仍是占位 0，Phase 6 接入计分后才有真实数字。
// ============================================================
void PanelDrawAll(const Font* font, const GameState* game)
{
    PanelDrawHoldBox(font, &game->hold);

    // 把"接下来 3 个方块"从袋子系统里取出来交给预览区。
    // 取不到（编号非法或队列异常）时用 0 兜底，保证界面不会因为数据问题而崩。
    int nextIds[BAG_PREVIEW_COUNT] = { 0, 0, 0 };
    for (int i = 0; i < BAG_PREVIEW_COUNT; i++)
    {
        int id = -1;
        if (bag_peek(&game->bag, i, &id))
        {
            nextIds[i] = id;
        }
    }
    PanelDrawNextBox(font, nextIds);

    // 统计区：Phase 6 起显示真实数据（分数 / 等级 / 消行数 / 连击）。
    // 连击少于 2 次时，PanelDrawStatsBox 内部会自动不显示连击提示，保持画面安静。
    PanelDrawStatsBox(font, game->score, game->level, game->totalLines, game->comboCount);
}

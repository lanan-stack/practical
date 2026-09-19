// ============================================================
// ui_screens.h —— 各个画面（标题 / 难度选择 / 暂停 / 结算）
// ------------------------------------------------------------
// 所属层次：界面层
//
// 职责：把这些"画面"画出来。它们只读游戏状态与最高分，不做任何规则判断。
//
// 屏幕状态机本身（谁跳转到谁）由 main.c 掌管，这里只负责画。
// ============================================================
#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "raylib.h"
#include "game_logic.h"
#include "highscore.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------- 标题画面 ----------
// 参数：font  —— 字体
//       scores —— 三档最高分（从存档读出来的）
//       time  —— 当前时间（秒），用来让"按 Enter 开始"这行字缓慢呼吸闪烁
void ScreenDrawTitle(const Font* font, const HighScores* scores, double time);

// ---------- 难度选择 ----------
// 参数：selected —— 当前选中的档位（0 轻松 / 1 标准 / 2 挑战）
//       time     —— 当前时间（秒），用来让选中项轻微上浮
void ScreenDrawDifficulty(const Font* font, int selected, double time);

// ---------- 暂停菜单（半透明遮罩盖在棋盘上）----------
// 参数：selected —— 0 继续 / 1 重开 / 2 回到标题
void ScreenDrawPauseOverlay(const Font* font, int selected);

// ---------- 结算画面（半透明遮罩盖在棋盘上）----------
// 参数：game      —— 本局状态（读分数 / 行数 / 等级 / 难度）
//       scores    —— 三档最高分
//       newRecord —— 本局是否破了纪录
//       selected  —— 0 重开 / 1 回到标题
void ScreenDrawGameOverOverlay(const Font* font, const GameState* game,
                               const HighScores* scores, bool newRecord, int selected);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // UI_SCREENS_H

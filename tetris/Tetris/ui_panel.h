// ============================================================
// ui_panel.h —— 右侧信息栏的绘制（HOLD / NEXT / 统计 / 连击）
// ------------------------------------------------------------
// 所属层次：界面层
//
// 职责：画右边那一竖条 —— 暂存区（HOLD）、预览区（NEXT，固定 3 个）、
//       统计区（分数 / 等级 / 行数）、以及右下角的连击提示。
//
// 边界：本模块只负责"画"。分数是多少、下一个方块是什么，都由调用方传入，
//       这里不做任何计算或规则判断。
// ============================================================
#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "raylib.h"
#include "hold.h"        // 暂存状态（Hold）
#include "bag.h"         // 预览数量 BAG_PREVIEW_COUNT
#include "game_logic.h"  // 游戏状态（只读）

// ---------- 画 HOLD（暂存区）框 ----------
// 参数：font —— 已加载好的中文字体
//       hold —— 暂存状态（Phase 5 起由游戏逻辑提供）
// 说明：槽里没有方块时只画一个空框。
void PanelDrawHoldBox(const Font* font, const Hold* hold);

// ---------- 画 NEXT（预览区）框 ----------
// 参数：font    —— 已加载好的中文字体
//       nextIds —— 接下来 3 个方块编号（调用方从袋子系统取好，界面层不自己算）
void PanelDrawNextBox(const Font* font, const int nextIds[BAG_PREVIEW_COUNT]);

// ---------- 画统计框（分数 / 等级 / 行数 / 连击）----------
// 参数：font  —— 字体
//       score —— 本局分数
//       level —— 当前等级
//       lines —— 已消行数
//       combo —— 当前连击次数（小于 2 时不显示连击提示，保持画面安静）
void PanelDrawStatsBox(const Font* font, int score, int level, int lines, int combo);

// ---------- 一次性画完整个信息栏 ----------
// 参数：font —— 字体
//       game —— 游戏状态（只读）：暂存区与预览区都从这里取真实数据
// 说明：统计区（分数 / 等级 / 行数）在 Phase 5 仍是占位的 0，
//       到 Phase 6 接入计分与等级后才显示真实数字。
void PanelDrawAll(const Font* font, const GameState* game);

#endif // UI_PANEL_H

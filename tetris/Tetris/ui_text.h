// ============================================================
// ui_text.h —— 界面上的全部文字，集中放这里
// ------------------------------------------------------------
// 所属层次：界面层配置（不依赖 raylib，只是字符串宏）
//
// 【为什么要单独建一个文件】
// raylib 加载字体时必须提前给出"要用到哪些字符"的清单，没列进去的字
// 会画成方框或问号。之前我们手工维护那份清单，结果连续漏了两个真的字
// （先是"空"，后是"块"），界面上就出现了"？"。
//
// 现在的做法是：**所有界面文字都定义在本文件里，字形清单由这些文字自动拼出来**
// （见最下方的 TEXT_ALL_GLYPHS）。以后新增文案只要写在本文件里，
// 字形就一定会被加载，不可能再漏字。手工清单那套彻底废弃。
//
// 维护约定：界面要显示的任何文字，都必须先在这里定义宏，再去代码里引用宏，
//           不允许在绘制代码里直接写中文字符串字面量。
// ============================================================
#ifndef UI_TEXT_H
#define UI_TEXT_H

// ---------- 标题 ----------
#define TEXT_TITLE          "俄罗斯方块"

// ---------- 信息栏标签 ----------
#define TEXT_HOLD           "暂存"
#define TEXT_NEXT           "预览"
#define TEXT_STATS          "统计"

// ---------- 统计项 ----------
#define TEXT_SCORE          "分数"
#define TEXT_LEVEL          "等级"
#define TEXT_LINES          "行数"

// ---------- 连击提示（带格式，供 sprintf 使用）----------
#define TEXT_COMBO_FORMAT   "连击 x%d"

// ---------- 底部操作提示 ----------
#define TEXT_HINT           "移动 A / D    旋转 W    软降 S    硬降 空格    暂存 C    暂停 ESC"

// ---------- 状态提示（Phase 5 先做一个最简单的"游戏结束"提示）----------
#define TEXT_GAME_OVER      "游戏结束"

// ---------- 标题画面（Phase 7）----------
#define TEXT_START_HINT     "按 Enter 开始"
#define TEXT_BEST_LIST      "最高分"

// ---------- 难度选择（Phase 7）----------
#define TEXT_DIFFICULTY_TITLE   "选择难度"
#define TEXT_DIFF_EASY          "轻松"
#define TEXT_DIFF_NORMAL        "标准"
#define TEXT_DIFF_HARD          "挑战"
#define TEXT_DIFF_SPEED_FORMAT  "封顶 %s 秒/格"

// ---------- 暂停菜单（Phase 7）----------
#define TEXT_PAUSED         "已暂停"
#define TEXT_MENU_RESUME    "继续"
#define TEXT_MENU_RESTART   "重开"
#define TEXT_MENU_BACK      "回到标题"
#define TEXT_MENU_HINT      "方向键选择    Enter 确认"

// ---------- 结算画面（Phase 7）----------
#define TEXT_RESULT_TITLE   "本局成绩"
#define TEXT_RESULT_SCORE   "分数"
#define TEXT_RESULT_LINES   "消行数"
#define TEXT_RESULT_LEVEL   "等级"
#define TEXT_RESULT_BEST    "本档最高分"
#define TEXT_NEW_RECORD     "新纪录！"

// ============================================================
// 字形清单：把所有界面文字拼在一起
// ------------------------------------------------------------
// main.c 会拿这整串字去问 raylib："这些字我都要用，请把它们都加载进字体。"
// 因为它是从上面那些文案拼出来的，所以**文案和字形永远不会脱节**。
// 注意：连击的格式串里含 "%d"，其中 d 属于 ASCII（数字由 ASCII 部分覆盖），
//       多带几个字符没有副作用。
// ============================================================
#define TEXT_ALL_GLYPHS     TEXT_TITLE TEXT_HOLD TEXT_NEXT TEXT_STATS \
                            TEXT_SCORE TEXT_LEVEL TEXT_LINES \
                            TEXT_COMBO_FORMAT TEXT_HINT TEXT_GAME_OVER \
                            TEXT_START_HINT TEXT_BEST_LIST TEXT_DIFFICULTY_TITLE \
                            TEXT_DIFF_EASY TEXT_DIFF_NORMAL TEXT_DIFF_HARD \
                            TEXT_DIFF_SPEED_FORMAT TEXT_PAUSED \
                            TEXT_MENU_RESUME TEXT_MENU_RESTART TEXT_MENU_BACK TEXT_MENU_HINT \
                            TEXT_RESULT_TITLE TEXT_RESULT_SCORE TEXT_RESULT_LINES \
                            TEXT_RESULT_LEVEL TEXT_RESULT_BEST TEXT_NEW_RECORD

#endif // UI_TEXT_H

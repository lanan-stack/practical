// ============================================================
// highscore.h —— 三档最高分存档（逻辑层）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib，可独立单元测试）
//
// 职责：把三档难度各自的最高分读出来、写回去。
// 设计原则（依据 project.md 第 2.7 节）：**任何异常都不许崩溃**。
// 文件不存在、是目录、内容乱写、数值超范围……一律视为"没有记录"，也就是 0 分。
//
// 【文件格式】纯文本，三行十进制整数：
//     第 1 行 = 轻松档最高分
//     第 2 行 = 标准档最高分
//     第 3 行 = 挑战档最高分
// 取这个格式是为了"人能读懂、能手改"。
//
// 【非法内容的处理】某一行的内容不合法时，**只把那一档记 0**，
// 另外两档的记录照常生效 —— 一行写坏不该连累其他档位。
// 注意"空行"也算不合法（记 0），绝不跳过：一旦跳过，三档的分数线就会错位，
// 而玩家几乎不可能发现这种错位。
// ============================================================
#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdbool.h>
#include "game_logic.h"   // Difficulty

// C / C++ 互操作：单元测试是 C++ 写的，加上 extern "C" 让两边函数命名一致。
#ifdef __cplusplus
extern "C" {
#endif

// 存档文件超过这个字节数就视为无记录（防止有人把别的大文件改名成存档）
#define HIGHSCORE_MAX_BYTES 4096

// 三档最高分
typedef struct HighScores
{
    int best[DIFFICULTY_COUNT];
} HighScores;

// ---------- 全部清零 ----------
void highscore_reset(HighScores* scores);

// ---------- 从文件读取 ----------
// 任何异常（打不开、是目录、内容非法、文件过大）都只会让对应档位变成 0，
// 绝不会崩溃，也不会留下半读半写的数据。
void highscore_load(HighScores* scores, const char* path);

// ---------- 写入文件（覆盖写）----------
// 返回：true = 成功；false = 失败（例如路径不可写）。
// 失败时调用方应继续正常运行，只是这一次成绩没存上。
bool highscore_save(const HighScores* scores, const char* path);

// ---------- 提交一次成绩 ----------
// 参数：scores —— 三档记录（可能被更新）
//       difficulty —— 这一局用的难度
//       score —— 本局得分
// 返回：true = 破纪录了（记录已更新）；false = 没破（记录不变）
// 说明：**同分不算破纪录**，否则结算画面会一直报"新纪录"。
bool highscore_submit(HighScores* scores, Difficulty difficulty, int score);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // HIGHSCORE_H

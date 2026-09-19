// ============================================================
// highscore.c —— 三档最高分存档的实现
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib）
//
// 读取流程：把整个文件读进内存（有大小上限）→ 按行切开 → 每行严格解析成整数。
// 写入流程：三行十进制整数，覆盖写。
// ============================================================
#include "highscore.h"
#include <stdio.h>
#include <string.h>

// ============================================================
// 内部：把一整行文字解析成"最高分"
// ------------------------------------------------------------
// 规则（宁可严格，也不要读出错的数据）：
//   · 先去掉行首行尾的空白（空格、制表符、回车）；
//   · 剩下的部分必须**全是十进制数字**，而且至少有一位；
//     所以 ""（空行）、"abc"、"-1"、" +123" 一律非法；
//   · 数值不能超过 int 的最大值（2147483647），超了就非法。
// 非法一律返回 0 —— 也就是"这一档没有记录"。
// ============================================================
static int highscore_parse_line(const char* line, int length)
{
    // ---------- 1. 掐掉首尾空白 ----------
    int begin = 0;
    int end = length;

    while (begin < end && (line[begin] == ' ' || line[begin] == '\t' || line[begin] == '\r'))
    {
        begin++;
    }
    while (end > begin && (line[end - 1] == ' ' || line[end - 1] == '\t' || line[end - 1] == '\r'))
    {
        end--;
    }

    if (begin >= end)
    {
        return 0;   // 空行：记 0（绝不跳过，避免三档分数线错位）
    }

    // ---------- 2. 逐位解析，边解析边检查范围 ----------
    // 用 long long 累加，一旦超过 int 上限就立刻判非法，
    // 这样超长数字串（几千位）也不会溢出。
    long long value = 0;
    const long long limit = 2147483647LL;   // int 的最大值

    for (int i = begin; i < end; i++)
    {
        const char ch = line[i];
        if (ch < '0' || ch > '9')
        {
            return 0;   // 出现了不是数字的字符（含正负号）
        }

        value = value * 10 + (ch - '0');
        if (value > limit)
        {
            return 0;   // 超出 int 范围
        }
    }

    return (int)value;
}

// ============================================================
// 内部：把整份文件的内容按行解析到三档里
// 返回：无（不合法的地方自然记 0）
// ============================================================
static void highscore_parse_all(HighScores* scores, const char* text, int length)
{
    highscore_reset(scores);

    int slot = 0;      // 当前正在解析第几档（0/1/2）
    int lineBegin = 0;

    for (int i = 0; i <= length; i++)
    {
        const bool atEnd = (i == length);
        if (!atEnd && text[i] != '\n')
        {
            continue;   // 还没到行尾
        }

        // 多余的行走忽略（只认前三行）
        if (slot < DIFFICULTY_COUNT)
        {
            scores->best[slot] = highscore_parse_line(text + lineBegin, i - lineBegin);
        }
        slot++;
        lineBegin = i + 1;
    }
}

// ============================================================
// 全部清零
// ============================================================
void highscore_reset(HighScores* scores)
{
    for (int i = 0; i < DIFFICULTY_COUNT; i++)
    {
        scores->best[i] = 0;
    }
}

// ============================================================
// 读取
// ------------------------------------------------------------
// 任何一步出问题都退化成"三档全 0"，绝不崩溃：
//   · 文件打不开（不存在、是目录、没有权限）→ 全 0
//   · 空文件 → 全 0
//   · 文件超过 4096 字节 → 全 0（防呆）
// ============================================================
void highscore_load(HighScores* scores, const char* path)
{
    highscore_reset(scores);

    if (path == NULL)
    {
        return;
    }

    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        return;   // 没有存档（第一次运行就是这样）
    }

    // 最多读 HIGHSCORE_MAX_BYTES + 1 个字节：多读一个字节是为了判断"是否超长"
    char buffer[HIGHSCORE_MAX_BYTES + 1];
    const size_t read = fread(buffer, 1, HIGHSCORE_MAX_BYTES + 1, file);
    fclose(file);

    if (read > HIGHSCORE_MAX_BYTES)
    {
        return;   // 文件过大：视为没有记录
    }

    highscore_parse_all(scores, buffer, (int)read);
}

// ============================================================
// 写入
// ------------------------------------------------------------
// 覆盖写三行十进制整数。任何一步失败都返回 false（调用方照常运行）。
// 用 "\r\n" 换行：Windows 记事本打开时最规矩。
// ============================================================
bool highscore_save(const HighScores* scores, const char* path)
{
    if (path == NULL)
    {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (file == NULL)
    {
        return false;   // 例如目标目录不存在、文件被占用
    }

    char buffer[64];
    bool ok = true;

    for (int i = 0; i < DIFFICULTY_COUNT; i++)
    {
        const int written = sprintf_s(buffer, sizeof(buffer), "%d\r\n", scores->best[i]);
        if (written <= 0 || fwrite(buffer, 1, (size_t)written, file) != (size_t)written)
        {
            ok = false;
            break;
        }
    }

    if (fclose(file) != 0)
    {
        ok = false;
    }
    return ok;
}

// ============================================================
// 提交一次成绩
// ------------------------------------------------------------
// 只有**严格更高**才算破纪录（同分不算），这样结算画面不会反复报"新纪录"。
// ============================================================
bool highscore_submit(HighScores* scores, Difficulty difficulty, int score)
{
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT)
    {
        return false;
    }
    if (score <= scores->best[difficulty])
    {
        return false;
    }

    scores->best[difficulty] = score;
    return true;
}

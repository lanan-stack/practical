// ============================================================
// test_highscore.cpp —— Phase 7 单元测试：三档最高分存档
// ------------------------------------------------------------
// 对应用例 P7-01 ~ P7-15（清单已由用户两轮审核通过）。
//
// 被测代码：highscore.h / highscore.c
//
// 【测试策略】所有用例都在临时目录 hs_test_tmp 下操作真实文件 ——
// 存档逻辑的核心就是"跟磁盘打交道时怎么处理各种异常"，
// 只有真的写文件、真的读文件，才测得出问题。
// ============================================================

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <direct.h>   // _mkdir

#include "highscore.h"
#include "game_logic.h"

// ============================================================
// 测试辅助
// ============================================================

static const char* kTempDir  = "hs_test_tmp";
static const char* kCasePath = "hs_test_tmp\\case.txt";
static const char* kDirPath  = "hs_test_tmp\\a_directory.txt";
static const char* kBadPath  = "hs_test_tmp\\no_such_dir\\a.txt";

// 确保临时目录存在（已存在时 _mkdir 返回失败，忽略即可）
static void EnsureTempDir()
{
    _mkdir(kTempDir);
}

// 把一段文字原样写进文件（用来构造各种"奇怪内容"的存档）
static void WriteRaw(const char* path, const std::string& text)
{
    FILE* file = nullptr;
    ASSERT_EQ(fopen_s(&file, path, "wb"), 0) << "无法创建测试文件";
    ASSERT_NE(file, nullptr);
    if (!text.empty())
    {
        ASSERT_EQ(fwrite(text.data(), 1, text.size(), file), text.size());
    }
    fclose(file);
}

// 读回整个文件的原始文本（用于检查"保存后的文件人能读懂"）
static std::string ReadRaw(const char* path)
{
    FILE* file = nullptr;
    if (fopen_s(&file, path, "rb") != 0 || file == nullptr)
    {
        return std::string();
    }

    std::string text;
    char buf[256];
    size_t n = 0;
    while ((n = fread(buf, 1, sizeof(buf), file)) > 0)
    {
        text.append(buf, n);
    }
    fclose(file);
    return text;
}

// 检查三档分数
static void ExpectScores(const HighScores& scores, int easy, int normal, int hard)
{
    EXPECT_EQ(scores.best[DIFFICULTY_EASY], easy);
    EXPECT_EQ(scores.best[DIFFICULTY_NORMAL], normal);
    EXPECT_EQ(scores.best[DIFFICULTY_HARD], hard);
}

// ============================================================
// A 组：读取与解析
// ============================================================

// P7-01 文件不存在 → 三档都是 0
TEST(Phase7HighScore, P7_01_FileMissing)
{
    EnsureTempDir();
    remove(kCasePath);

    HighScores scores;
    highscore_load(&scores, kCasePath);
    ExpectScores(scores, 0, 0, 0);
}

// P7-02 空文件 → 三档都是 0
TEST(Phase7HighScore, P7_02_EmptyFile)
{
    EnsureTempDir();
    WriteRaw(kCasePath, "");

    HighScores scores;
    highscore_load(&scores, kCasePath);
    ExpectScores(scores, 0, 0, 0);
}

// P7-03 正常三行
TEST(Phase7HighScore, P7_03_NormalFile)
{
    EnsureTempDir();
    WriteRaw(kCasePath, "1234\n567\n890\n");

    HighScores scores;
    highscore_load(&scores, kCasePath);
    ExpectScores(scores, 1234, 567, 890);
}

// P7-04 常见编辑痕迹：CRLF 换行、行尾空格、末尾空行
TEST(Phase7HighScore, P7_04_CommonEditingArtifacts)
{
    EnsureTempDir();
    WriteRaw(kCasePath, "1234\r\n567 \r\n  890  \n");

    HighScores scores;
    highscore_load(&scores, kCasePath);
    ExpectScores(scores, 1234, 567, 890);
}

// P7-05 空行算 0，绝不跳过（跳过会让三档分数线错位）
TEST(Phase7HighScore, P7_05_BlankLineMeansZero)
{
    EnsureTempDir();
    WriteRaw(kCasePath, "1234\n\n890\n");

    HighScores scores;
    highscore_load(&scores, kCasePath);
    // 空行必须当作该档 0，而不是跳过（跳过会让三档分数线错位）
    ExpectScores(scores, 1234, 0, 890);
}

// P7-06 非法内容：文字、负数、带正号 → 该档 0，其余照常
TEST(Phase7HighScore, P7_06_InvalidContentOnlyAffectsThatSlot)
{
    EnsureTempDir();

    const char* badLines[] = { "abc", "-1", "+123" };
    for (const char* bad : badLines)
    {
        const std::string text = std::string(bad) + "\n567\n890\n";
        WriteRaw(kCasePath, text);

        HighScores scores;
        highscore_load(&scores, kCasePath);
        ExpectScores(scores, 0, 567, 890);
    }
}

// P7-07 数值边界：2147483647 合法，2147483648 非法
TEST(Phase7HighScore, P7_07_IntegerBoundary)
{
    EnsureTempDir();

    WriteRaw(kCasePath, "2147483647\n0\n0\n");
    HighScores scores;
    highscore_load(&scores, kCasePath);
    EXPECT_EQ(scores.best[DIFFICULTY_EASY], 2147483647);

    WriteRaw(kCasePath, "2147483648\n0\n0\n");
    highscore_load(&scores, kCasePath);
    // 超出 int 范围必须判为非法
    ExpectScores(scores, 0, 0, 0);
}

// P7-08 行数不足与多余行
TEST(Phase7HighScore, P7_08_TooFewOrExtraLines)
{
    EnsureTempDir();

    // 只有一行：其余两档记 0
    WriteRaw(kCasePath, "1234\n");
    HighScores scores;
    highscore_load(&scores, kCasePath);
    ExpectScores(scores, 1234, 0, 0);

    // 多出来的行直接忽略
    WriteRaw(kCasePath, "1234\n567\n890\n999\n888\n");
    highscore_load(&scores, kCasePath);
    ExpectScores(scores, 1234, 567, 890);
}

// ============================================================
// B 组：写入与破纪录
// ============================================================

// P7-09 读写往返
TEST(Phase7HighScore, P7_09_SaveThenLoad)
{
    EnsureTempDir();

    HighScores out;
    out.best[DIFFICULTY_EASY] = 111;
    out.best[DIFFICULTY_NORMAL] = 222;
    out.best[DIFFICULTY_HARD] = 333;
    ASSERT_TRUE(highscore_save(&out, kCasePath));

    HighScores in;
    highscore_load(&in, kCasePath);
    ExpectScores(in, 111, 222, 333);
}

// P7-10 三档互不干扰
TEST(Phase7HighScore, P7_10_SlotsAreIndependent)
{
    EnsureTempDir();
    WriteRaw(kCasePath, "111\n222\n333\n");

    HighScores scores;
    highscore_load(&scores, kCasePath);

    ASSERT_TRUE(highscore_submit(&scores, DIFFICULTY_NORMAL, 999));
    ASSERT_TRUE(highscore_save(&scores, kCasePath));

    HighScores reloaded;
    highscore_load(&reloaded, kCasePath);
    // 只更新标准档，另外两档不该受影响
    ExpectScores(reloaded, 111, 999, 333);
}

// P7-11 破纪录判定：更高才算；更低与同分都不算
TEST(Phase7HighScore, P7_11_NewRecordRules)
{
    HighScores scores;
    highscore_reset(&scores);
    scores.best[DIFFICULTY_NORMAL] = 500;

    EXPECT_TRUE(highscore_submit(&scores, DIFFICULTY_NORMAL, 501)) << "更高应当破纪录";
    EXPECT_EQ(scores.best[DIFFICULTY_NORMAL], 501);

    EXPECT_FALSE(highscore_submit(&scores, DIFFICULTY_NORMAL, 400)) << "更低不该破纪录";
    EXPECT_EQ(scores.best[DIFFICULTY_NORMAL], 501) << "更低时记录不该被改写";

    EXPECT_FALSE(highscore_submit(&scores, DIFFICULTY_NORMAL, 501)) << "同分不算破纪录";
    EXPECT_EQ(scores.best[DIFFICULTY_NORMAL], 501);
}

// P7-12 保存后的文件内容：三行纯数字、顺序正确（人能读懂、能手改）
TEST(Phase7HighScore, P7_12_SavedFileIsHumanReadable)
{
    EnsureTempDir();

    HighScores scores;
    scores.best[DIFFICULTY_EASY] = 11;
    scores.best[DIFFICULTY_NORMAL] = 22;
    scores.best[DIFFICULTY_HARD] = 33;
    ASSERT_TRUE(highscore_save(&scores, kCasePath));

    const std::string text = ReadRaw(kCasePath);
    EXPECT_NE(text.find("11"), std::string::npos) << "文件里应当能看到分数本身";
    EXPECT_NE(text.find("22"), std::string::npos);
    EXPECT_NE(text.find("33"), std::string::npos);

    // 顺序必须与档位一致：11 在前、33 在后
    EXPECT_LT(text.find("11"), text.find("22"));
    EXPECT_LT(text.find("22"), text.find("33"));

    // 而且这份文件必须能被自己重新读回来（格式自洽）
    HighScores reloaded;
    highscore_load(&reloaded, kCasePath);
    ExpectScores(reloaded, 11, 22, 33);
}

// P7-13 保存失败：路径不可写时返回 false，不崩溃
TEST(Phase7HighScore, P7_13_SaveFailureIsReported)
{
    EnsureTempDir();

    HighScores scores;
    highscore_reset(&scores);

    EXPECT_FALSE(highscore_save(&scores, kBadPath))
        << "目录不存在时应当返回失败，而不是崩溃";
}

// ============================================================
// C 组：文件本身的异常
// ============================================================

// P7-14 同名目录：视为无记录
TEST(Phase7HighScore, P7_14_PathIsADirectory)
{
    EnsureTempDir();
    remove(kDirPath);
    _mkdir(kDirPath);

    HighScores scores;
    highscore_load(&scores, kDirPath);
    // 路径是目录时应当视为没有记录
    ExpectScores(scores, 0, 0, 0);
}

// P7-15 超长文件：视为无记录
TEST(Phase7HighScore, P7_15_FileTooLarge)
{
    EnsureTempDir();

    // 造一个远超过 4096 字节的文件
    std::string huge;
    for (int i = 0; i < 5000; i++)
    {
        huge += '7';
    }
    WriteRaw(kCasePath, huge);

    HighScores scores;
    highscore_load(&scores, kCasePath);
    ExpectScores(scores, 0, 0, 0);
}

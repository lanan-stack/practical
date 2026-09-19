// ============================================================
// test_audio.cpp —— Phase 8 单元测试：音效合成
// ------------------------------------------------------------
// 对应用例 P8-01 ~ P8-12（清单已由用户两轮审核通过）。
//
// 被测代码：audio.h / audio.c 里的**纯计算部分**（audio_render 等）。
// 播放部分依赖声卡，不在这里测 —— 它由"用户亲耳验收"把关。
//
// 【怎么"量"声音】听感没法直接断言，但可以量化三件事：
//   · 过零次数（一段时间内波形穿过零点几次）—— 频率的代理指标，用来判断音高高低；
//   · 峰值 —— 判断响度、是否有声音、是否爆音；
//   · 相邻样本的最大跳变 —— 判断有没有"咔哒"声。
// ============================================================

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "audio.h"

// ============================================================
// 测试辅助
// ============================================================

// 合成一个音效，返回样本数组
static std::vector<short> Render(AudioSound sound, int semitones = 0)
{
    const int capacity = AUDIO_SAMPLE_RATE;   // 预留 1 秒，够任何音效
    std::vector<short> buffer(capacity, 0);

    const int count = audio_render(sound, semitones, buffer.data(), capacity);
    buffer.resize(count > 0 ? count : 0);
    return buffer;
}

// 统计 [from, to) 区间内波形穿过零点的次数（频率的代理指标）
static int CountZeroCrossings(const std::vector<short>& v, int from, int to)
{
    if (to > (int)v.size()) { to = (int)v.size(); }

    int crossings = 0;
    for (int i = from + 1; i < to; i++)
    {
        const bool before = (v[i - 1] >= 0);
        const bool now = (v[i] >= 0);
        if (before != now)
        {
            crossings++;
        }
    }
    return crossings;
}

// 统计 [from, to) 区间内绝对值的最大值（0~1 之间）
static double PeakOf(const std::vector<short>& v, int from, int to)
{
    if (to > (int)v.size()) { to = (int)v.size(); }

    int peak = 0;
    for (int i = from; i < to; i++)
    {
        const int a = std::abs((int)v[i]);
        if (a > peak) { peak = a; }
    }
    return (double)peak / 32767.0;
}

// 统计 [from, to) 区间内绝对值的平均值（0~1 之间）
static double AverageAmplitude(const std::vector<short>& v, int from, int to)
{
    if (to > (int)v.size()) { to = (int)v.size(); }
    if (to <= from) { return 0.0; }

    long long sum = 0;
    for (int i = from; i < to; i++)
    {
        sum += std::abs((int)v[i]);
    }
    return (double)sum / (double)(to - from) / 32767.0;
}

// ============================================================
// A 组：音高与升调
// ============================================================

// P8-01 五声音阶序列
TEST(Phase8Audio, P8_01_PentatonicScale)
{
    const int expected[AUDIO_COMBO_STEPS] = { 0, 2, 4, 7, 9, 12 };
    for (int i = 0; i < AUDIO_COMBO_STEPS; i++)
    {
        EXPECT_EQ(AUDIO_PENTATONIC[i], expected[i]) << "第 " << i << " 级不对";
    }
}

// P8-02 升调映射与循环
TEST(Phase8Audio, P8_02_ComboPitchMappingAndLoop)
{
    const int expected[AUDIO_COMBO_STEPS + 2] = { 0, 2, 4, 7, 9, 12, 0, 2 };
    for (int n = 1; n <= AUDIO_COMBO_STEPS + 2; n++)
    {
        EXPECT_EQ(audio_combo_semitones(n), expected[n - 1])
            << "第 " << n << " 次消行的升调半音数不对";
    }
}

// P8-03 首次消行不升调（非法输入也按基础音处理）
TEST(Phase8Audio, P8_03_FirstClearUsesBasePitch)
{
    EXPECT_EQ(audio_combo_semitones(1), 0);
    EXPECT_EQ(audio_combo_semitones(0), 0) << "0 或负数应当按第 1 次处理";
    EXPECT_EQ(audio_combo_semitones(-5), 0);
}

// P8-04 半音换算：+12 半音正好是两倍频
TEST(Phase8Audio, P8_04_SemitoneRatio)
{
    EXPECT_NEAR(audio_semitone_ratio(0), 1.0, 1e-9);
    EXPECT_NEAR(audio_semitone_ratio(12), 2.0, 2e-3) << "高八度应当是两倍频";
    EXPECT_NEAR(audio_semitone_ratio(24), 4.0, 1e-2);
    EXPECT_NEAR(audio_semitone_ratio(-12), 0.5, 1e-3);
}

// P8-05 消 1/2/3 行的音高依次升高
TEST(Phase8Audio, P8_05_ClearSoundsGoUp)
{
    const int window = AUDIO_SAMPLE_RATE * 150 / 1000;   // 取前 150 毫秒比较

    const std::vector<short> one = Render(SOUND_CLEAR_1);
    const std::vector<short> two = Render(SOUND_CLEAR_2);
    const std::vector<short> three = Render(SOUND_CLEAR_3);

    const int c1 = CountZeroCrossings(one, 0, window);
    const int c2 = CountZeroCrossings(two, 0, window);
    const int c3 = CountZeroCrossings(three, 0, window);

    EXPECT_LT(c1, c2) << "消 2 行的音高应当高于消 1 行";
    EXPECT_LT(c2, c3) << "消 3 行的音高应当高于消 2 行";
}

// ============================================================
// B 组：波形与包络
// ============================================================

// P8-06 采样数与时长一致
TEST(Phase8Audio, P8_06_SampleCountMatchesDuration)
{
    for (int s = 0; s < SOUND_COUNT; s++)
    {
        const AudioSound sound = (AudioSound)s;
        const std::vector<short> samples = Render(sound);
        const int expected = (int)(audio_sound_seconds(sound) * (double)AUDIO_SAMPLE_RATE);

        EXPECT_GT(expected, 0) << "音效 " << s << " 的时长不合法";
        EXPECT_EQ((int)samples.size(), expected) << "音效 " << s << " 的样本数不对";
    }
}

// P8-08 包络：起音从零开始、结尾衰减回零，且首尾一段的平均幅度明显低于峰值
// ------------------------------------------------------------
// 说明：不能简单地拿"前 2% 的峰值"去比 —— 起音只有 5 毫秒，
//       对 0.4 秒的音效来说 2% 已经有 8 毫秒，把满音量的部分也框进来了。
//       真正要守的两条是：首尾样本必须接近零（不会"啪"地开始/结束），
//       以及首尾一小段的平均幅度远低于峰值（确实有淡入淡出）。
TEST(Phase8Audio, P8_08_EnvelopeFadesInAndOut)
{
    for (int s = 0; s < SOUND_COUNT; s++)
    {
        const std::vector<short> v = Render((AudioSound)s);
        ASSERT_FALSE(v.empty());

        const int n = (int)v.size();
        const double whole = PeakOf(v, 0, n);
        ASSERT_GT(whole, 0.0);

        // 1) 首尾样本必须接近零
        EXPECT_LT(std::abs((int)v[0]) / 32767.0, whole * 0.01)
            << "音效 " << s << " 的起音不是从零开始，会有咔哒声";
        EXPECT_LT(std::abs((int)v[n - 1]) / 32767.0, whole * 0.01)
            << "音效 " << s << " 的结尾没有回到零，会有咔哒声";

        // 2) 首尾一小段的平均幅度明显低于峰值
        const int window = (n / 100) > 0 ? (n / 100) : 1;   // 约 1%

        EXPECT_LT(AverageAmplitude(v, 0, window), whole * 0.6)
            << "音效 " << s << " 起音太快，没有淡入";
        EXPECT_LT(AverageAmplitude(v, n - window, n), whole * 0.6)
            << "音效 " << s << " 结尾没有淡出";
    }
}

// P8-08 不爆音、不咔哒
TEST(Phase8Audio, P8_09_NoClippingNoClicks)
{
    for (int s = 0; s < SOUND_COUNT; s++)
    {
        const std::vector<short> v = Render((AudioSound)s);
        ASSERT_FALSE(v.empty());

        EXPECT_LE(PeakOf(v, 0, (int)v.size()), 1.0) << "音效 " << s << " 爆音了";

        int maxJump = 0;
        for (size_t i = 1; i < v.size(); i++)
        {
            const int jump = std::abs((int)v[i] - (int)v[i - 1]);
            if (jump > maxJump) { maxJump = jump; }
        }
        EXPECT_LT((double)maxJump / 32767.0, 0.25)
            << "音效 " << s << " 的波形跳变过大，听感上会有咔哒声";
    }
}

// P8-09 每个音效都必须有声音（防止"静音"伪装成"不爆音"）
TEST(Phase8Audio, P8_10_EverySoundIsAudible)
{
    for (int s = 0; s < SOUND_COUNT; s++)
    {
        const std::vector<short> v = Render((AudioSound)s);
        ASSERT_FALSE(v.empty());
        EXPECT_GT(PeakOf(v, 0, (int)v.size()), 0.1)
            << "音效 " << s << " 几乎没有声音";
    }
}

// ============================================================
// C 组：音效特征
// ============================================================

// P8-10 硬降是下滑音：前半段的过零率明显高于后半段
TEST(Phase8Audio, P8_11_HardDropSweepsDown)
{
    const std::vector<short> v = Render(SOUND_HARD_DROP);
    ASSERT_FALSE(v.empty());

    const int half = (int)v.size() / 2;
    const int first = CountZeroCrossings(v, 0, half);
    const int second = CountZeroCrossings(v, half, (int)v.size());

    EXPECT_GT((double)first, (double)second * 1.5)
        << "前半段频率应当明显高于后半段（下滑音）；若反了，说明写成了上滑";
}

// P8-11 消 4 行是上行琶音：后半段的过零率高于前半段
TEST(Phase8Audio, P8_12_ClearFourIsRisingArpeggio)
{
    const std::vector<short> v = Render(SOUND_CLEAR_4);
    ASSERT_FALSE(v.empty());

    const int third = (int)v.size() / 3;
    const int first = CountZeroCrossings(v, 0, third);
    const int last = CountZeroCrossings(v, third * 2, (int)v.size());

    EXPECT_GT((double)last, (double)first * 1.2)
        << "琶音应当往上走：后半段频率要高于前半段";
}

// ============================================================
// D 组：确定性
// ============================================================

// P8-12 同一音效连续合成两次，样本数据必须完全一致
TEST(Phase8Audio, P8_13_RenderIsDeterministic)
{
    for (int s = 0; s < SOUND_COUNT; s++)
    {
        const std::vector<short> a = Render((AudioSound)s);
        const std::vector<short> b = Render((AudioSound)s);

        ASSERT_EQ(a.size(), b.size());
        for (size_t i = 0; i < a.size(); i++)
        {
            if (a[i] != b[i])
            {
                FAIL() << "音效 " << s << " 第 " << i << " 个样本两次合成不一致";
            }
        }
    }
}

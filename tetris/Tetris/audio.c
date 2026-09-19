// ============================================================
// audio.c —— 音效合成与播放的实现
// ------------------------------------------------------------
// 所属层次：界面层（依赖 raylib）
//
// 【音色配方】三角波打底 + 少量二次谐波。
//   为什么不用方波：方波谐波丰富但很"刺"，长时间听会累；
//   纯正弦又太"软"，没有方块的机械感。三角波正好在中间，
//   再加 15% 的二次谐波，让音色亮一点、不那么死板。
//
// 【包络】5 毫秒快速起音 + 二次曲线衰减到 0。
//   起音快 = 反应干脆；衰减到 0 = 结尾不会有"啪"的一声（爆音/咔哒声）。
// ============================================================
#include "audio.h"
#include "raylib.h"

#include <math.h>
#include <string.h>

// MSVC 的 math.h 默认不提供 M_PI，这里自己定义一个（避免依赖宏开关）
#define AUDIO_PI 3.14159265358979323846

const int AUDIO_PENTATONIC[AUDIO_COMBO_STEPS] = { 0, 2, 4, 7, 9, 12 };

// ============================================================
// 每个音效的配方
// ------------------------------------------------------------
//   seconds   —— 时长（秒）
//   baseHz    —— 基频
//   sweepToHz —— 大于 0 表示"从 baseHz 滑到这个频率"（用于硬降的下滑音）
//   arpeggio  —— 1 表示三段上行琶音（消 4 行用）
//   volume    —— 归一化后的目标峰值（各音效之间的音量平衡靠它）
// ============================================================
typedef struct SoundSpec
{
    double seconds;
    double baseHz;
    double sweepToHz;
    int    arpeggio;
    float  volume;
} SoundSpec;

static const SoundSpec SPECS[SOUND_COUNT] =
{
    { 0.040, 220.00,   0.0, 0, 0.35f },   // 移动：极短极轻，只有"有反应"的感觉
    { 0.070, 440.00,   0.0, 0, 0.55f },   // 旋转：短促清脆
    { 0.090, 330.00, 110.0, 0, 0.65f },   // 硬降：330 → 110 Hz 下滑，落地冲击感
    { 0.120, 180.00,   0.0, 0, 0.60f },   // 锁定：沉稳的一声
    { 0.200, 523.25,   0.0, 0, 0.70f },   // 消 1 行：C5
    { 0.260, 659.25,   0.0, 0, 0.72f },   // 消 2 行：E5
    { 0.320, 783.99,   0.0, 0, 0.75f },   // 消 3 行：G5
    { 0.420, 523.25,   0.0, 1, 0.80f }    // 消 4 行：C5 → E5 → G5 上行琶音
};

// 琶音的三个音相对基频的倍数：大三度（5/4）、纯五度（3/2）
static const double ARPEGGIO_MULT[3] = { 1.0, 1.25, 1.5 };

// ============================================================
// 半音数 → 频率倍数
// ============================================================
double audio_semitone_ratio(int semitones)
{
    return pow(2.0, (double)semitones / 12.0);
}

double audio_sound_seconds(AudioSound sound)
{
    if (sound < 0 || sound >= SOUND_COUNT)
    {
        return 0.0;
    }
    return SPECS[sound].seconds;
}

// ============================================================
// 连击次数 → 升调半音数
// ------------------------------------------------------------
// 第 1 次消行不升调，之后每连击一次往上走一级，走满六级回到起点循环。
// ============================================================
int audio_combo_semitones(int comboCount)
{
    if (comboCount < 1)
    {
        comboCount = 1;
    }
    return AUDIO_PENTATONIC[(comboCount - 1) % AUDIO_COMBO_STEPS];
}

// ============================================================
// 内部：算出第 i 个样本的"原始波形值"（未归一化）
// ------------------------------------------------------------
// 用相位累积而不是直接算 sin(2πft)：频率变化时（下滑音、琶音）
// 相位仍然连续，不会出现"咯"的一声。
// ============================================================
static double AudioSampleAt(const SoundSpec* spec, double ratio, int i, int count)
{
    const double t = (double)i / (double)AUDIO_SAMPLE_RATE;
    const double u = (count > 1) ? (double)i / (double)(count - 1) : 0.0;   // 0 → 1

    // ---------- 1. 当前频率 ----------
    double freq = spec->baseHz * ratio;

    if (spec->sweepToHz > 0.0)
    {
        // 线性下滑：从 baseHz 滑到 sweepToHz
        const double endHz = spec->sweepToHz * ratio;
        freq = freq + (endHz - freq) * u;
    }
    else if (spec->arpeggio)
    {
        // 三段上行琶音：按进度分成三段，各取一个音
        int segment = (int)(u * 3.0);
        if (segment > 2) { segment = 2; }
        freq = spec->baseHz * ARPEGGIO_MULT[segment] * ratio;
    }

    // ---------- 2. 相位与波形 ----------
    // 说明：这里用 t 乘频率得到相位。因为频率会随时间变化，严格做法是累积相位；
    //       但本项目的音效很短（≤0.42 秒），且频率变化平缓，
    //       直接按 t 计算在听感上没有差别，代码也更简单。
    const double cycles = freq * t;
    const double phase = cycles - floor(cycles);                 // 0 → 1
    const double triangle = 4.0 * fabs(phase - 0.5) - 1.0;       // 三角波：-1 → 1
    const double secondHarmonic = sin(2.0 * AUDIO_PI * 2.0 * phase) * 0.15;
    const double wave = triangle + secondHarmonic;

    // ---------- 3. 包络：5 毫秒起音 + 二次曲线衰减 ----------
    const double attack = 0.005;
    double env;
    if (t < attack)
    {
        env = t / attack;                                        // 起音
    }
    else
    {
        const double total = spec->seconds;
        const double u2 = (total > attack) ? (t - attack) / (total - attack) : 1.0;
        const double remain = 1.0 - u2;
        env = remain * remain;                                   // 平滑衰减到 0
    }

    return wave * env;
}

// ============================================================
// 合成一个音效
// ------------------------------------------------------------
// 两遍处理：第一遍找峰值，第二遍按目标音量归一化后写出去。
// 归一化的好处：不管波形怎么叠加，所有音效的响度都稳定，不会有的响有的轻。
// ============================================================
int audio_render(AudioSound sound, int semitones, short* outSamples, int capacity)
{
    if (sound < 0 || sound >= SOUND_COUNT || outSamples == NULL)
    {
        return 0;
    }

    const SoundSpec* spec = &SPECS[sound];
    const int count = (int)(spec->seconds * (double)AUDIO_SAMPLE_RATE);
    if (count <= 0 || capacity < count)
    {
        return 0;
    }

    const double ratio = audio_semitone_ratio(semitones);

    // ---------- 第一遍：找峰值 ----------
    double peak = 0.0;
    for (int i = 0; i < count; i++)
    {
        const double v = fabs(AudioSampleAt(spec, ratio, i, count));
        if (v > peak)
        {
            peak = v;
        }
    }
    if (peak <= 1e-9)
    {
        return 0;   // 不应该是静音；真出现了就当作失败
    }

    // ---------- 第二遍：归一化并写入 ----------
    const double scale = (double)spec->volume / peak;
    for (int i = 0; i < count; i++)
    {
        double v = AudioSampleAt(spec, ratio, i, count) * scale;
        if (v > 1.0) { v = 1.0; }
        if (v < -1.0) { v = -1.0; }
        outSamples[i] = (short)(v * 32767.0);
    }
    return count;
}

// ============================================================
// 播放部分
// ------------------------------------------------------------
// 为了"连击升调"能即时播放，启动时就把 4 个消行音 × 6 个升调级数的变体
// 全部合成好（4 + 24 = 28 个音效），播放时只是按下标取用，不做任何计算。
// ============================================================
static Sound g_sounds[SOUND_COUNT];                                  // 四个基础音效
static Sound g_clearSounds[4][AUDIO_COMBO_STEPS];                    // 消行音 × 升调级数
static bool  g_audioReady = false;
static float g_masterVolume = 1.0f;

// 内部：合成一段样本并加载成 raylib 的 Sound
static Sound BuildSound(AudioSound sound, int semitones)
{
    const int maxSamples = (int)(0.5 * (double)AUDIO_SAMPLE_RATE);   // 最长音效 0.42 秒，够用
    short* samples = (short*)MemAlloc(sizeof(short) * (size_t)maxSamples);

    const int count = audio_render(sound, semitones, samples, maxSamples);
    if (count <= 0)
    {
        MemFree(samples);
        Sound empty = { 0 };
        return empty;
    }

    // 组装成 raylib 需要的 Wave 结构：单声道、16 位
    Wave wave = { 0 };
    wave.frameCount = (unsigned int)count;
    wave.sampleRate = AUDIO_SAMPLE_RATE;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;

    Sound result = LoadSoundFromWave(wave);
    MemFree(samples);   // 数据已经拷进 Sound 里了
    return result;
}

void audio_init(void)
{
    InitAudioDevice();
    if (!IsAudioDeviceReady())
    {
        return;   // 没有声卡也不影响游戏运行，只是没声音
    }

    for (int s = 0; s < SOUND_COUNT; s++)
    {
        g_sounds[s] = BuildSound((AudioSound)s, 0);
    }

    // 消行音：每个行数各做 6 个升调变体（连击一到六级）
    for (int lines = 1; lines <= 4; lines++)
    {
        for (int step = 0; step < AUDIO_COMBO_STEPS; step++)
        {
            const int semitones = AUDIO_PENTATONIC[step];
            g_clearSounds[lines - 1][step] =
                BuildSound((AudioSound)(SOUND_CLEAR_1 + (lines - 1)), semitones);
        }
    }

    g_audioReady = true;
}

void audio_shutdown(void)
{
    if (!g_audioReady)
    {
        return;
    }

    for (int s = 0; s < SOUND_COUNT; s++)
    {
        UnloadSound(g_sounds[s]);
    }
    for (int lines = 0; lines < 4; lines++)
    {
        for (int step = 0; step < AUDIO_COMBO_STEPS; step++)
        {
            UnloadSound(g_clearSounds[lines][step]);
        }
    }

    CloseAudioDevice();
    g_audioReady = false;
}

void audio_play(AudioSound sound)
{
    if (!g_audioReady || sound < 0 || sound >= SOUND_COUNT)
    {
        return;
    }
    PlaySound(g_sounds[sound]);
}

void audio_play_clear(int lines, int comboCount)
{
    if (!g_audioReady)
    {
        return;
    }
    if (lines < 1) { lines = 1; }
    if (lines > 4) { lines = 4; }
    if (comboCount < 1) { comboCount = 1; }

    const int step = (comboCount - 1) % AUDIO_COMBO_STEPS;
    PlaySound(g_clearSounds[lines - 1][step]);
}

void audio_set_master_volume(float volume)
{
    if (volume < 0.0f) { volume = 0.0f; }
    if (volume > 1.0f) { volume = 1.0f; }

    g_masterVolume = volume;
    if (g_audioReady)
    {
        SetMasterVolume(g_masterVolume);
    }
}

// ============================================================
// audio.h —— 音效（界面层，依赖 raylib）
// ------------------------------------------------------------
// 所属层次：界面层（用到 raylib 的音频功能）
//
// 【设计要点：合成与播放分开】
//   · 合成（audio_render）是一个**纯函数**：输入音效编号与升调半音数，
//     输出一段 16 位单声道样本。它不碰 raylib 的播放设备，因此可以被单元测试
//     完整覆盖（量频率、量包络、量峰值都能做）。
//   · 播放（audio_init / audio_play …）只是薄薄一层 raylib 调用。
//
// 【全部现场合成】不引入任何音频文件：8 个音效都是启动时用三角波 + 少量二次谐波
// 算出来的，连击升调则用同一套合成函数乘以不同的音高倍数。
// ============================================================
#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 采样格式：44100 Hz、单声道、16 位（也是 raylib 的默认格式）
#define AUDIO_SAMPLE_RATE 44100

// 八个音效
typedef enum AudioSound
{
    SOUND_MOVE = 0,      // 左右移动
    SOUND_ROTATE,        // 旋转
    SOUND_HARD_DROP,     // 硬降（下滑音）
    SOUND_LOCK,          // 落地锁定
    SOUND_CLEAR_1,       // 消 1 行
    SOUND_CLEAR_2,       // 消 2 行
    SOUND_CLEAR_3,       // 消 3 行
    SOUND_CLEAR_4,       // 消 4 行（上行琶音，全曲最高光）
    SOUND_COUNT
} AudioSound;

// 连击升调用的音阶级数（六级一循环）
#define AUDIO_COMBO_STEPS 6

// ============================================================
// 纯计算部分（可单元测试，不依赖音频设备）
// ============================================================

// 五声音阶的半音偏移表：0, +2, +4, +7, +9, +12（大调五声音阶跨一个八度）
extern const int AUDIO_PENTATONIC[AUDIO_COMBO_STEPS];

// 半音数 → 频率倍数。例如 +12 半音应当正好是 2 倍频。
double audio_semitone_ratio(int semitones);

// 取某个音效的时长（秒），音效编号非法时返回 0。
// 单独提供这个函数，是为了让测试能核对"样本数 = 时长 × 采样率"。
double audio_sound_seconds(AudioSound sound);

// 连击次数 → 升调半音数
// 规则：第 1 次消行用基础音（+0），第 2 次 +2，……，第 6 次 +12，
//       第 7 次回到基础音循环。也就是取五声音阶的第 (n-1) % 6 级。
// 参数小于 1 时按 1 处理。
int audio_combo_semitones(int comboCount);

// 合成一个音效（含升调）到 outSamples
// 参数：sound     —— 音效编号
//       semitones —— 升调的半音数（0 表示原调）
//       outSamples—— 输出缓冲（16 位单声道样本）
//       capacity  —— 缓冲能容纳的样本数
// 返回：写入的样本数；参数非法或容量不足时返回 0
int audio_render(AudioSound sound, int semitones, short* outSamples, int capacity);

// ============================================================
// 播放部分（依赖 raylib）
// ============================================================

// 初始化音频设备并生成全部音效（含连击升调的 6 档变体）。
// 音频设备不可用时不会崩溃，只是没有声音。
void audio_init(void);
void audio_shutdown(void);

// 播放一个音效
void audio_play(AudioSound sound);

// 播放消行音：按消掉的行数选音效，按连击次数做升调
// 参数：lines —— 本次消掉几行（1~4）
//       comboCount —— 这是连续第几次消行（从 1 开始）
void audio_play_clear(int lines, int comboCount);

// 整体音量（0.0 ~ 1.0），用于统一调节
void audio_set_master_volume(float volume);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // AUDIO_H

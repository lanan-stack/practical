// ============================================================
// input_repeat.h —— 按键长按连发（DAS / ARR）（逻辑层）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，不依赖 raylib，可独立单元测试）
//
// 为什么把它放在逻辑层而不是界面代码里：
//   "按住左键方块滑得快不快"完全由时间决定，放在这里就能精确到 0.01 秒地测，
//   而不是只能靠手感去猜。界面层每帧把"这个键现在是否按住"告诉它，
//   它回答"这一帧该不该动一格"。
//
// 工作方式（以按住左键为例）：
//   按下瞬间   → 立刻移动一格（返回 1）
//   继续按住   → 等待 INPUT_DAS_SECONDS（这段时间内不重复移动）
//   等待结束后 → 每 INPUT_ARR_SECONDS 移动一格
//   松开       → 计时清零，下次按下从头开始
// ============================================================
#ifndef INPUT_REPEAT_H
#define INPUT_REPEAT_H

#ifdef __cplusplus
extern "C" {
#endif

// 一个按键的连发状态
//   wasHeld      —— 上一帧是否按住（用来识别"按下瞬间"）
//   heldSeconds  —— 本次按住已经持续了多久
//   repeating    —— 是否已经进入连发阶段
//   repeatTimer  —— 连发阶段里，距离下一次移动累计了多久
typedef struct InputRepeat
{
    int    wasHeld;
    double heldSeconds;
    int    repeating;
    double repeatTimer;
} InputRepeat;

// 初始化（未按下状态）
void input_repeat_init(InputRepeat* repeat);

// 每帧调用一次
// 参数：repeat  —— 连发状态
//       held    —— 这一帧该键是否被按住（0/1）
//       dt      —— 距上一帧经过的秒数
// 返回：这一帧应当触发几次动作（0 或 1）
int input_repeat_update(InputRepeat* repeat, int held, double dt);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // INPUT_REPEAT_H

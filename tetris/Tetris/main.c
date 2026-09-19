// ============================================================
// main.c —— 程序入口（Phase 7：完整的游戏流程）
// ------------------------------------------------------------
// 所属层次：界面层。职责：
//   1. 掌管"屏幕状态机"：标题 → 难度选择 → 游戏中 → 暂停 / 结算；
//   2. 把键盘输入翻译成逻辑层的操作；
//   3. 每帧把时间喂给逻辑层推进，然后把状态画出来。
//
// 界面层不判断任何游戏规则，规则全在 game_logic 里。
//
// 【本阶段新增】标题画面、难度选择、暂停菜单、结算画面、三档最高分存档。
// 【还没做】Phase 8 的音效。
// ============================================================

#include "raylib.h"

#include "ui_config.h"
#include "ui_text.h"
#include "ui_board.h"
#include "ui_panel.h"
#include "ui_screens.h"
#include "game_logic.h"
#include "input_repeat.h"
#include "highscore.h"
#include "audio.h"

// ---------- 断开输入法所需的唯一一个系统函数 ----------
// 本来应该 #include <windows.h>，但那个头文件会定义 DrawText、LoadImage 之类的宏，
// 还会声明同名的 CloseWindow，和 raylib 的函数名直接打架（编译不过）。
// 我们只需要一个函数，所以自己声明它，绕开整个头文件。
//   ImmAssociateContext(HWND, HIMC)：把输入法上下文与窗口解除关联；
//   传 NULL 就是断开输入法 —— 之后字母键直达游戏，不再被输入法截走。
#pragma comment(lib, "imm32.lib")
__declspec(dllimport) void* __stdcall ImmAssociateContext(void* hWnd, void* hImc);

// 屏幕状态
typedef enum Screen
{
    SCREEN_TITLE = 0,
    SCREEN_DIFFICULTY,
    SCREEN_PLAYING,
    SCREEN_PAUSED,
    SCREEN_GAME_OVER
} Screen;

// ============================================================
// 关闭输入法（IME）对游戏窗口的接管
// ------------------------------------------------------------
// 中文输入法打开时，字母键会先被输入法截走：表现出来就是
// "WASD 按了没反应、按 C 屏幕上还冒出候选框"，而方向键却一切正常
// （方向键不属于输入法的处理范围）。本游戏不需要输入任何文字，
// 所以直接把窗口的输入法上下文断开，所有按键就都直达游戏了。
// ============================================================
static void DisableImeForWindow(void)
{
    void* hwnd = GetWindowHandle();   // raylib 返回的就是窗口句柄（Windows 上即 HWND）
    if (hwnd != NULL)
    {
        ImmAssociateContext(hwnd, NULL);
    }
}

// ============================================================
// 加载界面字体（ASCII + 界面用到的全部中文）
// ------------------------------------------------------------
// 需要哪些字形由 ui_text.h 的 TEXT_ALL_GLYPHS 自动拼出，
// 所以界面文案与字形清单永远不会脱节。
// ============================================================
static Font LoadUiFont(void)
{
    int cnCount = 0;
    int* cnCodepoints = LoadCodepoints(TEXT_ALL_GLYPHS, &cnCount);

    const int asciiFirst = 32;
    const int asciiCount = 95;
    int total = asciiCount + cnCount;

    int* codepoints = (int*)MemAlloc(sizeof(int) * (size_t)total);
    for (int i = 0; i < asciiCount; i++)
    {
        codepoints[i] = asciiFirst + i;
    }
    for (int i = 0; i < cnCount; i++)
    {
        codepoints[asciiCount + i] = cnCodepoints[i];
    }

    Font font = LoadFontEx(FONT_PATH, FONT_BASE_SIZE, codepoints, total);

    MemFree(codepoints);
    UnloadCodepoints(cnCodepoints);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    if (!IsFontValid(font))
    {
        TraceLog(LOG_WARNING, "字体加载失败，请确认输出目录下有 resources/simhei.ttf");
    }
    return font;
}

// ============================================================
// 游戏中的标题带与底部提示
// ============================================================
static void DrawTitleBand(const Font* font)
{
    const char* title = TEXT_TITLE;
    Vector2 size = MeasureTextEx(*font, title, (float)TITLE_FONT_SIZE, 2.0f);
    float x = ((float)WINDOW_WIDTH - size.x) / 2.0f;
    DrawTextEx(*font, title, (Vector2){ x, (float)TITLE_Y },
               (float)TITLE_FONT_SIZE, 2.0f, COLOR_CAPTION);
}

static void DrawHintLine(const Font* font)
{
    const char* hint = TEXT_HINT;
    Vector2 size = MeasureTextEx(*font, hint, (float)HINT_FONT_SIZE, 1.0f);
    float x = ((float)WINDOW_WIDTH - size.x) / 2.0f;
    DrawTextEx(*font, hint, (Vector2){ x, (float)HINT_Y },
               (float)HINT_FONT_SIZE, 1.0f, COLOR_HINT);
}

// ============================================================
// 通用：菜单的上下选择
// ------------------------------------------------------------
// 返回 true 表示"按下了确认键"。选择项会循环（到底了回到第一个）。
// ============================================================
static bool HandleMenuKeys(int* index, int count)
{
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        *index = (*index + count - 1) % count;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        *index = (*index + 1) % count;
    }
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
}

// ============================================================
// 通用：开一局新游戏
// ------------------------------------------------------------
// 重开时先释放上一局的资源（袋子里的队列是 malloc 出来的），再重新初始化。
// 用当前时间当随机种子：每次开局的方块顺序都不一样。
// ============================================================
static void StartNewGame(GameState* game, bool* gameReady, Difficulty difficulty,
                         InputRepeat* leftRepeat, InputRepeat* rightRepeat)
{
    if (*gameReady)
    {
        game_destroy(game);
    }

    game_init(game, (unsigned int)(GetTime() * 1000.0) + 1000u);
    game_set_difficulty(game, difficulty);

    *gameReady = true;
    input_repeat_init(leftRepeat);
    input_repeat_init(rightRepeat);
}

// ============================================================
// 程序入口
// ============================================================
int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetExitKey(0);            // 取消"按 ESC 直接退出"：ESC 现在用来暂停
    SetTargetFPS(60);
    DisableImeForWindow();    // 断开中文输入法，保证字母键直达游戏
    Font uiFont = LoadUiFont();
    audio_init();             // 生成全部音效（含连击升调的六档变体）

    // 读入三档最高分（任何异常都会退化成全 0，不影响游戏运行）
    HighScores scores;
    highscore_load(&scores, HIGHSCORE_FILE_PATH);

    Screen screen = SCREEN_TITLE;
    int menuIndex = 0;
    Difficulty chosenDifficulty = DIFFICULTY_NORMAL;

    GameState game;
    bool gameReady = false;        // 游戏状态是否已经初始化过（决定要不要释放资源）
    bool recordSubmitted = false;  // 本局成绩是否已经提交过（避免重复写入存档）
    bool newRecord = false;

    InputRepeat leftRepeat;
    InputRepeat rightRepeat;
    input_repeat_init(&leftRepeat);
    input_repeat_init(&rightRepeat);

    while (!WindowShouldClose())
    {
        const double dt = (double)GetFrameTime();

        // ==================== 状态机：处理输入 ====================
        switch (screen)
        {
            // ---------- 标题画面 ----------
            case SCREEN_TITLE:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                {
                    menuIndex = (int)chosenDifficulty;   // 记住上次选的档位
                    screen = SCREEN_DIFFICULTY;
                }
                break;

            // ---------- 难度选择 ----------
            case SCREEN_DIFFICULTY:
            {
                // 数字键 1/2/3 可以直接选，省得上下挪
                if (IsKeyPressed(KEY_ONE))   { menuIndex = 0; }
                if (IsKeyPressed(KEY_TWO))   { menuIndex = 1; }
                if (IsKeyPressed(KEY_THREE)) { menuIndex = 2; }

                if (HandleMenuKeys(&menuIndex, DIFFICULTY_COUNT))
                {
                    chosenDifficulty = (Difficulty)menuIndex;
                    StartNewGame(&game, &gameReady, chosenDifficulty, &leftRepeat, &rightRepeat);
                    recordSubmitted = false;
                    newRecord = false;
                    screen = SCREEN_PLAYING;
                }
                break;
            }

            // ---------- 游戏中 ----------
            case SCREEN_PLAYING:
            {
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    menuIndex = 0;
                    screen = SCREEN_PAUSED;
                    break;
                }

                // 记下"操作之前"的状态：用来判断这一帧发生了什么，好放对应的音效
                const int  prevTotalLines = game.totalLines;
                const int  prevPieceId = game.pieceId;
                const bool wasClearing = game.isClearing;
                bool suppressLockSound = false;   // 硬降已经响过"落地音"，不必再响"锁定音"

                // 方向键与 WASD 同时支持
                const int leftHeld = (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) ? 1 : 0;
                const int rightHeld = (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) ? 1 : 0;
                const int softHeld = (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) ? 1 : 0;

                // 左右移动（含长按连发）
                const int leftMoves = input_repeat_update(&leftRepeat, leftHeld, dt);
                const int rightMoves = input_repeat_update(&rightRepeat, rightHeld, dt);
                for (int i = 0; i < leftMoves; i++)
                {
                    if (game_move(&game, -1)) { audio_play(SOUND_MOVE); }
                }
                for (int i = 0; i < rightMoves; i++)
                {
                    if (game_move(&game, +1)) { audio_play(SOUND_MOVE); }
                }

                // 旋转 / 硬降 / 暂存（按一下触发一次）
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_X))
                {
                    if (game_rotate(&game, true)) { audio_play(SOUND_ROTATE); }
                }
                if (IsKeyPressed(KEY_Z))
                {
                    if (game_rotate(&game, false)) { audio_play(SOUND_ROTATE); }
                }
                if (IsKeyPressed(KEY_SPACE))
                {
                    game_hard_drop(&game);
                    audio_play(SOUND_HARD_DROP);
                    suppressLockSound = true;
                }
                if (IsKeyPressed(KEY_C))
                {
                    game_hold_piece(&game);
                }

                // 推进时间（软降按住时由逻辑层把下落速度提到 10 倍）
                game_update(&game, dt, softHeld);

                // ---------- 根据状态变化放音效 ----------
                // 消行：行数变多 → 按"消了几行 + 当前连击"播放（连击会自动升调）
                if (game.totalLines != prevTotalLines)
                {
                    audio_play_clear(game.totalLines - prevTotalLines, game.comboCount);
                }
                // 锁定：换了新方块、且上一帧不在消行阶段、而且不是刚硬降过
                else if (game.pieceId != prevPieceId && !wasClearing && !suppressLockSound)
                {
                    audio_play(SOUND_LOCK);
                }

                // 顶出 → 进入结算画面
                if (game.gameOver)
                {
                    menuIndex = 0;
                    screen = SCREEN_GAME_OVER;
                }
                break;
            }

            // ---------- 暂停菜单 ----------
            case SCREEN_PAUSED:
            {
                if (HandleMenuKeys(&menuIndex, 3))
                {
                    if (menuIndex == 0)
                    {
                        input_repeat_init(&leftRepeat);   // 回到游戏时清掉连发计时
                        input_repeat_init(&rightRepeat);
                        screen = SCREEN_PLAYING;
                    }
                    else if (menuIndex == 1)
                    {
                        StartNewGame(&game, &gameReady, chosenDifficulty, &leftRepeat, &rightRepeat);
                        recordSubmitted = false;
                        newRecord = false;
                        screen = SCREEN_PLAYING;
                    }
                    else
                    {
                        screen = SCREEN_TITLE;
                    }
                }
                // 暂停界面里再按一次 ESC 也可以直接继续
                else if (IsKeyPressed(KEY_ESCAPE))
                {
                    screen = SCREEN_PLAYING;
                }
                break;
            }

            // ---------- 结算画面 ----------
            case SCREEN_GAME_OVER:
            {
                // 进入本画面的第一帧：提交成绩（只做一次）
                if (!recordSubmitted)
                {
                    recordSubmitted = true;
                    newRecord = highscore_submit(&scores, game.difficulty, game.score);
                    if (newRecord)
                    {
                        // 只有破纪录才需要写文件；写失败也不影响游戏继续运行
                        highscore_save(&scores, HIGHSCORE_FILE_PATH);
                    }
                }

                if (HandleMenuKeys(&menuIndex, 2))
                {
                    if (menuIndex == 0)
                    {
                        StartNewGame(&game, &gameReady, chosenDifficulty, &leftRepeat, &rightRepeat);
                        recordSubmitted = false;
                        newRecord = false;
                        screen = SCREEN_PLAYING;
                    }
                    else
                    {
                        screen = SCREEN_TITLE;
                    }
                }
                break;
            }
        }

        // ==================== 绘制 ====================
        BeginDrawing();
        ClearBackground(COLOR_WINDOW_BG);

        switch (screen)
        {
            case SCREEN_TITLE:
                ScreenDrawTitle(&uiFont, &scores, GetTime());
                break;

            case SCREEN_DIFFICULTY:
                ScreenDrawDifficulty(&uiFont, menuIndex, GetTime());
                break;

            case SCREEN_PLAYING:
                DrawTitleBand(&uiFont);
                BoardDrawGame(&game);
                PanelDrawAll(&uiFont, &game);
                DrawHintLine(&uiFont);
                break;

            case SCREEN_PAUSED:
                DrawTitleBand(&uiFont);
                BoardDrawGame(&game);
                PanelDrawAll(&uiFont, &game);
                ScreenDrawPauseOverlay(&uiFont, menuIndex);
                break;

            case SCREEN_GAME_OVER:
                DrawTitleBand(&uiFont);
                BoardDrawGame(&game);
                PanelDrawAll(&uiFont, &game);
                ScreenDrawGameOverOverlay(&uiFont, &game, &scores, newRecord, menuIndex);
                break;
        }

        EndDrawing();
    }

    if (gameReady)
    {
        game_destroy(&game);
    }
    audio_shutdown();   // 释放音效与音频设备
    UnloadFont(uiFont);
    CloseWindow();
    return 0;
}

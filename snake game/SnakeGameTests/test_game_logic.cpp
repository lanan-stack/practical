// ============================================================
// P3 单元测试：核心游戏逻辑层（MSTest 原生单元测试）
// 覆盖：初始化、tick 移动、转向与输入缓冲（180° 拦截）、
//       撞墙/撞自身、追尾合法、吃食物增长与再生成、
//       计分与加速档位/上限、死亡态、棋盘占满、销毁
// ============================================================
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// C 头文件需要用 extern "C" 包起来，避免 C++ 名称修饰
extern "C"
{
#include "config.h"
#include "DCList.h"
#include "snake_list.h"
#include "game_logic.h"
}

TEST_CLASS(GameLogicTests)
{
public:
    // 工具：把蛇身替换成自定义形状（rows/cols 按"头→尾"顺序）
    static void SetSnake(GameState* g, const int rows[], const int cols[], int n)
    {
        SnakeDestroy(&g->snake);
        SnakeInit(&g->snake);
        for (int i = n - 1; i >= 0; i--)
        {
            SnakePushHead(&g->snake, SnakePosEncode(rows[i], cols[i]));
        }
    }

    // 工具：把食物放到蛇头正前方一个 tick 吃掉（每吃一个 +1 分）
    static void Feed(GameState* g)
    {
        int hr = SnakePosRow(SnakeHead(&g->snake));
        int hc = SnakePosCol(SnakeHead(&g->snake));
        int dr = (GameDirection(g) == DIR_DOWN) ? 1 : (GameDirection(g) == DIR_UP) ? -1 : 0;
        int dc = (GameDirection(g) == DIR_RIGHT) ? 1 : (GameDirection(g) == DIR_LEFT) ? -1 : 0;
        g->foodRow = hr + dr;
        g->foodCol = hc + dc;
        GameTick(g);
    }

    // 用例 1：初始化——3 格居中、头朝右、速度 8、分数 0、存活、进度 0、队列空
    TEST_METHOD(Init_BoardCenter_ThreeSegmentsRight)
    {
        GameState g;
        GameInit(&g, 1);
        Assert::AreEqual(3, SnakeLength(&g.snake));
        Assert::AreEqual(SnakePosEncode(15, 16), SnakeHead(&g.snake));
        Assert::AreEqual(SnakePosEncode(15, 15), SnakeAt(&g.snake, 1));
        Assert::AreEqual(SnakePosEncode(15, 14), SnakeAt(&g.snake, 2));
        Assert::IsTrue(GameDirection(&g) == DIR_RIGHT);
        Assert::AreEqual(SNAKE_INIT_SPEED, GameSpeed(&g));
        Assert::AreEqual(0, GameScore(&g));
        Assert::AreEqual(0, GameFoodProgress(&g));
        Assert::IsTrue(GameIsAlive(&g));
        Assert::AreEqual(0, g.queueCount);
        GameDestroy(&g);
    }

    // 用例 2：同种子食物位置可复现，且坐标在界内、不与蛇身重叠
    TEST_METHOD(Init_SameSeed_SameFoodNotOnSnake)
    {
        GameState a, b;
        GameInit(&a, 42);
        GameInit(&b, 42);
        Assert::AreEqual(a.foodRow, b.foodRow);
        Assert::AreEqual(a.foodCol, b.foodCol);
        Assert::IsTrue(a.foodRow >= 0 && a.foodRow < SNAKE_BOARD_SIZE);
        Assert::IsTrue(a.foodCol >= 0 && a.foodCol < SNAKE_BOARD_SIZE);
        Assert::IsTrue(SnakeContains(&a.snake, SnakePosEncode(a.foodRow, a.foodCol)) == 0);
        GameDestroy(&a);
        GameDestroy(&b);
    }

    // 用例 3：直行一格——头前移、长度不变、旧尾巴释放
    TEST_METHOD(Tick_MoveRight_SlidesOneCell)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = g.foodCol = -1; // 本用例不涉及食物
        GameTick(&g);
        Assert::AreEqual(SnakePosEncode(15, 17), SnakeHead(&g.snake));
        Assert::AreEqual(3, SnakeLength(&g.snake));
        Assert::IsTrue(SnakeContains(&g.snake, SnakePosEncode(15, 14)) == 0);
        Assert::IsTrue(GameIsAlive(&g));
        GameDestroy(&g);
    }

    // 用例 4：转向在消费该输入的 tick 生效——按 D 后本 tick 即向下移动
    TEST_METHOD(Turn_Down_AppliesOnConsumingTick)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = g.foodCol = -1;
        GameQueueDirection(&g, DIR_DOWN);
        GameTick(&g);
        Assert::IsTrue(GameDirection(&g) == DIR_DOWN);
        Assert::AreEqual(SnakePosEncode(16, 16), SnakeHead(&g.snake));
        GameTick(&g);
        Assert::AreEqual(SnakePosEncode(17, 16), SnakeHead(&g.snake));
        GameDestroy(&g);
    }

    // 用例 5：直接 180° 掉头被拦截——朝右按左，队列空、方向不变
    TEST_METHOD(Reversal_Direct_Ignored)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = g.foodCol = -1;
        GameQueueDirection(&g, DIR_LEFT);
        Assert::AreEqual(0, g.queueCount);
        Assert::IsTrue(GameDirection(&g) == DIR_RIGHT);
        GameTick(&g);
        Assert::AreEqual(SnakePosEncode(15, 17), SnakeHead(&g.snake)); // 仍向右
        GameDestroy(&g);
    }

    // 用例 6：队列内的 180° 也被拦截——先按上再按下，下被拒绝
    TEST_METHOD(Reversal_AfterQueued_Ignored)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = g.foodCol = -1;
        GameQueueDirection(&g, DIR_UP);
        GameQueueDirection(&g, DIR_DOWN); // 相对队尾 UP 是 180°，拒绝
        Assert::AreEqual(1, g.queueCount);
        GameTick(&g);
        Assert::IsTrue(GameDirection(&g) == DIR_UP);
        Assert::AreEqual(SnakePosEncode(14, 16), SnakeHead(&g.snake));
        GameDestroy(&g);
    }

    // 用例 7：输入排队——一个 tick 只消费一个，按序执行（先上后左）
    TEST_METHOD(Queue_ConsumesOnePerTick_InOrder)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = g.foodCol = -1;
        GameQueueDirection(&g, DIR_UP);
        GameQueueDirection(&g, DIR_LEFT);
        Assert::AreEqual(2, g.queueCount);

        GameTick(&g); // 消费 UP
        Assert::IsTrue(GameDirection(&g) == DIR_UP);
        Assert::AreEqual(1, g.queueCount);
        Assert::AreEqual(SnakePosEncode(14, 16), SnakeHead(&g.snake));

        GameTick(&g); // 消费 LEFT
        Assert::IsTrue(GameDirection(&g) == DIR_LEFT);
        Assert::AreEqual(0, g.queueCount);
        Assert::AreEqual(SnakePosEncode(14, 15), SnakeHead(&g.snake));
        GameDestroy(&g);
    }

    // 用例 8：队列满丢弃最旧；被丢弃后首元素若与当前方向 180° 则消费时忽略
    TEST_METHOD(Queue_Full_DropsOldest_AndReversalAtConsume)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = g.foodCol = -1;
        GameQueueDirection(&g, DIR_UP);
        GameQueueDirection(&g, DIR_LEFT);
        GameQueueDirection(&g, DIR_DOWN); // 队列满：[UP, LEFT, DOWN]
        GameQueueDirection(&g, DIR_RIGHT); // 满→丢 UP，队列变 [LEFT, DOWN, RIGHT]
        Assert::AreEqual(3, g.queueCount);
        Assert::IsTrue(g.queue[g.queueHead] == DIR_LEFT); // UP 确实被丢弃

        GameTick(&g); // 消费 LEFT，但与当前方向 RIGHT 180°，防御性忽略
        Assert::IsTrue(GameDirection(&g) == DIR_RIGHT);
        Assert::AreEqual(SnakePosEncode(15, 17), SnakeHead(&g.snake));
        Assert::AreEqual(2, g.queueCount);

        GameTick(&g); // 消费 DOWN
        Assert::IsTrue(GameDirection(&g) == DIR_DOWN);
        Assert::AreEqual(SnakePosEncode(16, 17), SnakeHead(&g.snake));

        GameTick(&g); // 消费 RIGHT
        Assert::IsTrue(GameDirection(&g) == DIR_RIGHT);
        Assert::AreEqual(SnakePosEncode(16, 18), SnakeHead(&g.snake));
        Assert::AreEqual(0, g.queueCount);
        GameDestroy(&g);
    }

    // 用例 9：撞墙死亡——蛇身保持原样
    TEST_METHOD(Wall_Hit_Dies_SnakeUnchanged)
    {
        GameState g;
        GameInit(&g, 1);
        int rows[] = { 0, 1, 2 };
        int cols[] = { 5, 5, 5 };
        SetSnake(&g, rows, cols, 3);
        g.dir = DIR_UP;
        g.foodRow = g.foodCol = -1;
        GameTick(&g);
        Assert::IsTrue(GameIsAlive(&g) == 0);
        Assert::AreEqual(3, SnakeLength(&g.snake));
        Assert::AreEqual(SnakePosEncode(0, 5), SnakeHead(&g.snake));
        GameDestroy(&g);
    }

    // 用例 10：撞自身（中间身体）死亡——蛇身保持原样
    TEST_METHOD(Self_Hit_MiddleBody_Dies_SnakeUnchanged)
    {
        GameState g;
        GameInit(&g, 1);
        // 头(5,5)朝上，身体绕成 C 形，正上方(4,5)是中间身体（非尾巴）
        int rows[] = { 5, 5, 4, 4, 4, 5 };
        int cols[] = { 5, 4, 4, 5, 6, 6 };
        SetSnake(&g, rows, cols, 6);
        g.dir = DIR_UP;
        g.foodRow = g.foodCol = -1;
        GameTick(&g);
        Assert::IsTrue(GameIsAlive(&g) == 0);
        Assert::AreEqual(6, SnakeLength(&g.snake));
        Assert::AreEqual(SnakePosEncode(5, 5), SnakeHead(&g.snake));
        GameDestroy(&g);
    }

    // 用例 11：追尾合法——头进入"本 tick 刚腾空"的尾巴格不死，长度保持不变
    TEST_METHOD(Move_IntoVacatedTail_Legal)
    {
        GameState g;
        GameInit(&g, 1);
        // 头(5,5)朝上： (5,5)->(5,4)->(4,4)->尾(4,5)
        int rows[] = { 5, 5, 4, 4 };
        int cols[] = { 5, 4, 4, 5 };
        SetSnake(&g, rows, cols, 4);
        g.dir = DIR_UP;
        g.foodRow = g.foodCol = -1;
        GameTick(&g);
        Assert::IsTrue(GameIsAlive(&g));
        Assert::AreEqual(4, SnakeLength(&g.snake)); // 尾巴弹出、头补入同格，长度不变
        Assert::AreEqual(SnakePosEncode(4, 5), SnakeHead(&g.snake));
        Assert::AreEqual(SnakePosEncode(5, 5), SnakeAt(&g.snake, 1)); // 原头成为第一节身体
        Assert::AreEqual(SnakePosEncode(4, 4), SnakeAt(&g.snake, 3)); // 倒数第二节成为新尾
        GameDestroy(&g);
    }

    // 用例 12：吃食物——长度 +1、分数 +1、旧尾巴保留（增长）
    TEST_METHOD(Eat_Food_GrowsAndScores)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = 15;
        g.foodCol = 17; // 蛇头正前方
        GameTick(&g);
        Assert::AreEqual(4, SnakeLength(&g.snake));
        Assert::AreEqual(SnakePosEncode(15, 17), SnakeHead(&g.snake));
        Assert::AreEqual(1, GameScore(&g));
        Assert::AreEqual(SnakePosEncode(15, 14), SnakeAt(&g.snake, 3)); // 旧尾保留
        Assert::IsTrue(GameIsAlive(&g));
        GameDestroy(&g);
    }

    // 用例 13：吃食物后重新生成——新食物在界内且不与蛇身重叠
    TEST_METHOD(Eat_Food_RespawnsFoodNotOnSnake)
    {
        GameState g;
        GameInit(&g, 1);
        g.foodRow = 15;
        g.foodCol = 17;
        GameTick(&g);
        Assert::IsTrue(g.foodRow >= 0 && g.foodRow < SNAKE_BOARD_SIZE);
        Assert::IsTrue(g.foodCol >= 0 && g.foodCol < SNAKE_BOARD_SIZE);
        Assert::IsTrue(SnakeContains(&g.snake, SnakePosEncode(g.foodRow, g.foodCol)) == 0);
        GameDestroy(&g);
    }

    // 用例 14：每 5 个食物加速一档——4 个时速度不变、进度 4；第 5 个速度 +1、进度归零
    TEST_METHOD(Speed_FiveFoodsPerLevel_ProgressReset)
    {
        GameState g;
        GameInit(&g, 1);
        for (int i = 0; i < 4; i++) Feed(&g);
        Assert::AreEqual(4, GameScore(&g));
        Assert::AreEqual(SNAKE_INIT_SPEED, GameSpeed(&g));
        Assert::AreEqual(4, GameFoodProgress(&g));
        Assert::AreEqual(0, GameSpeedLevel(&g));

        Feed(&g); // 第 5 个
        Assert::AreEqual(5, GameScore(&g));
        Assert::AreEqual(SNAKE_INIT_SPEED + 1, GameSpeed(&g));
        Assert::AreEqual(0, GameFoodProgress(&g));
        Assert::AreEqual(1, GameSpeedLevel(&g));
        GameDestroy(&g);
    }

    // 用例 15：加速上限——分数到 50 后速度封顶 18、档位封顶 10
    TEST_METHOD(Speed_Capped_AtMax)
    {
        GameState g;
        GameInit(&g, 1);
        // 构造"已吃 45 个"的中间状态（分数 45、速度 17），蛇身远离墙壁
        int rows[] = { 10, 10, 10 };
        int cols[] = { 20, 19, 18 };
        SetSnake(&g, rows, cols, 3);
        g.dir = DIR_RIGHT;
        g.score = 45;
        g.speed = 17;
        g.foodRow = g.foodCol = -1;

        for (int i = 0; i < 6; i++) Feed(&g); // 吃到 51 分
        Assert::AreEqual(51, GameScore(&g));
        Assert::AreEqual(SNAKE_MAX_SPEED, GameSpeed(&g));
        Assert::AreEqual(10, GameSpeedLevel(&g));
        Assert::AreEqual(1, GameFoodProgress(&g));
        GameDestroy(&g);
    }

    // 用例 16：死亡后 tick 无操作——蛇身、分数、食物都不变
    TEST_METHOD(Dead_Tick_NoOp)
    {
        GameState g;
        GameInit(&g, 1);
        int rows[] = { 0, 1, 2 };
        int cols[] = { 5, 5, 5 };
        SetSnake(&g, rows, cols, 3);
        g.dir = DIR_UP;
        g.foodRow = 20;
        g.foodCol = 20;
        GameTick(&g); // 撞墙死
        Assert::IsTrue(GameIsAlive(&g) == 0);
        int len = SnakeLength(&g.snake);
        int head = SnakeHead(&g.snake);
        int score = GameScore(&g);

        GameTick(&g); // 死亡后再 tick
        Assert::AreEqual(len, SnakeLength(&g.snake));
        Assert::AreEqual(head, SnakeHead(&g.snake));
        Assert::AreEqual(score, GameScore(&g));
        Assert::AreEqual(20, g.foodRow);
        Assert::AreEqual(20, g.foodCol);
        GameDestroy(&g);
    }

    // 用例 17：棋盘被蛇占满时无法生成食物（返回 -1）
    TEST_METHOD(BoardFull_SpawnFood_ReturnsNoFood)
    {
        GameState g;
        GameInit(&g, 1);
        SnakeDestroy(&g.snake);
        SnakeInit(&g.snake);
        for (int r = 0; r < SNAKE_BOARD_SIZE; r++)
        {
            for (int c = 0; c < SNAKE_BOARD_SIZE; c++)
            {
                SnakePushHead(&g.snake, SnakePosEncode(r, c));
            }
        }
        Assert::AreEqual(900, SnakeLength(&g.snake));
        GameSpawnFood(&g);
        Assert::AreEqual(-1, g.foodRow);
        Assert::AreEqual(-1, g.foodCol);
        GameDestroy(&g);
    }

    // 用例 18：销毁后蛇链表指针为空
    TEST_METHOD(Destroy_Game_FreesSnake)
    {
        GameState g;
        GameInit(&g, 1);
        Assert::IsNotNull(g.snake.nodes);
        GameDestroy(&g);
        Assert::IsNull(g.snake.nodes);
    }

    // 用例 19：四面墙都撞死——上/下/左/右边界越界均判死亡且蛇身不变
    TEST_METHOD(Wall_Hit_AllFourSides_Die)
    {
        // 每种墙：蛇头贴墙、身体向内延伸
        const int headRows[4] = { 0, 29, 5, 5 };
        const int headCols[4] = { 5, 5, 0, 29 };
        const Direction dirs[4] = { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

        for (int k = 0; k < 4; k++)
        {
            GameState g;
            GameInit(&g, 1);
            // 身体向内排两节（身体方向与移动方向相反）
            int rows[3], cols[3];
            rows[0] = headRows[k]; cols[0] = headCols[k];
            rows[1] = headRows[k] - (dirs[k] == DIR_UP ? -1 : dirs[k] == DIR_DOWN ? 1 : 0);
            cols[1] = headCols[k] - (dirs[k] == DIR_LEFT ? -1 : dirs[k] == DIR_RIGHT ? 1 : 0);
            rows[2] = rows[1] + (rows[1] - rows[0]);
            cols[2] = cols[1] + (cols[1] - cols[0]);
            SetSnake(&g, rows, cols, 3);
            g.dir = dirs[k];
            g.foodRow = g.foodCol = -1;

            GameTick(&g);
            Assert::IsTrue(GameIsAlive(&g) == 0);
            Assert::AreEqual(3, SnakeLength(&g.snake));
            Assert::AreEqual(SnakePosEncode(headRows[k], headCols[k]), SnakeHead(&g.snake));
            GameDestroy(&g);
        }
    }

    // 用例 20：吃食物同时撞到身体——防御分支，判定死亡且蛇身不变
    // （正常流程食物不会生成在蛇身上，此用例锁定"万一出现"时的安全行为）
    TEST_METHOD(Eat_WithBodyCollision_Dies)
    {
        GameState g;
        GameInit(&g, 1);
        // 头(5,5)朝右，(5,6)是中间身体（第 6 节，非尾巴）
        int rows[] = { 5, 5, 4, 4, 4, 5, 6 };
        int cols[] = { 5, 4, 4, 5, 6, 6, 6 };
        SetSnake(&g, rows, cols, 7);
        g.dir = DIR_RIGHT;
        g.foodRow = 5;
        g.foodCol = 6; // 把食物人为放到身体格上

        GameTick(&g);
        Assert::IsTrue(GameIsAlive(&g) == 0);
        Assert::AreEqual(7, SnakeLength(&g.snake));
        Assert::AreEqual(SnakePosEncode(5, 5), SnakeHead(&g.snake));
        GameDestroy(&g);
    }

    // 用例 21：死亡后输入被忽略——队列保持空、方向不变
    TEST_METHOD(Dead_Game_IgnoresQueue)
    {
        GameState g;
        GameInit(&g, 1);
        int rows[] = { 0, 1, 2 };
        int cols[] = { 5, 5, 5 };
        SetSnake(&g, rows, cols, 3);
        g.dir = DIR_UP;
        g.foodRow = g.foodCol = -1;
        GameTick(&g); // 撞墙死
        Assert::IsTrue(GameIsAlive(&g) == 0);

        GameQueueDirection(&g, DIR_DOWN);
        Assert::AreEqual(0, g.queueCount);
        Assert::IsTrue(GameDirection(&g) == DIR_UP);
        GameDestroy(&g);
    }

    // 用例 22：长蛇（50 节）时食物生成仍避开蛇身、坐标在界内
    TEST_METHOD(SpawnFood_LongSnake_AvoidsBody)
    {
        GameState g;
        GameInit(&g, 1);
        SnakeDestroy(&g.snake);
        SnakeInit(&g.snake);
        // 第 15 行放满 30 格 + 第 14 行放 20 格 = 50 节
        for (int c = 0; c < SNAKE_BOARD_SIZE; c++)
        {
            SnakePushHead(&g.snake, SnakePosEncode(15, c));
        }
        for (int c = SNAKE_BOARD_SIZE - 1; c >= 10; c--)
        {
            SnakePushHead(&g.snake, SnakePosEncode(14, c));
        }
        Assert::AreEqual(50, SnakeLength(&g.snake));

        GameSpawnFood(&g);
        Assert::IsTrue(g.foodRow >= 0 && g.foodRow < SNAKE_BOARD_SIZE);
        Assert::IsTrue(g.foodCol >= 0 && g.foodCol < SNAKE_BOARD_SIZE);
        Assert::IsTrue(SnakeContains(&g.snake, SnakePosEncode(g.foodRow, g.foodCol)) == 0);
        GameDestroy(&g);
    }

    // 用例 23：同种子多轮食物序列可复现——两局游戏连吃 3 个食物，每次新食物位置一致
    TEST_METHOD(SameSeed_FoodSequence_Reproducible)
    {
        GameState a, b;
        GameInit(&a, 7);
        GameInit(&b, 7);
        for (int i = 0; i < 3; i++)
        {
            Feed(&a);
            Feed(&b);
            Assert::AreEqual(a.foodRow, b.foodRow);
            Assert::AreEqual(a.foodCol, b.foodCol);
        }
        Assert::AreEqual(3, GameScore(&a));
        Assert::AreEqual(GameScore(&a), GameScore(&b));
        GameDestroy(&a);
        GameDestroy(&b);
    }
};

// ============================================================
// P6 单元测试：最高分存档（HighScoreLoad / HighScoreSave）
// 覆盖：读写往返、覆盖旧值、空文件/垃圾/负数/溢出/多余字符、
//       文件不存在、NULL 路径、非法路径、负分保存失败
// ============================================================
#include <string>
#include <cstdio>
#include <process.h>   // GetCurrentProcessId（生成唯一临时文件名）
#include <climits>     // INT_MAX（边界值用例）
#include <direct.h>    // _mkdir / _rmdir（目录路径用例）
#include <sys/stat.h>  // _chmod / _S_IREAD / _S_IWRITE（只读文件用例）
#include <io.h>        // _chmod 的补充声明

TEST_CLASS(HighScoreTests)
{
public:
    // 工具：生成带进程号的唯一临时文件名（落在测试运行目录，可写）
    static std::string TempPath(const char* name)
    {
        return std::string("highscore_") + name + "_" + std::to_string(_getpid()) + ".txt";
    }

    // 工具：尽力删除临时文件（删不掉不影响用例结果）
    static void Cleanup(const std::string& path)
    {
        remove(path.c_str());
    }

    // 工具：生成唯一临时目录名（用于"路径是目录"用例）
    static std::string TempDir(const char* name)
    {
        return std::string("highscore_dir_") + name + "_" + std::to_string(_getpid());
    }

    // 用例 1：文件不存在 → 读取返回 0
    TEST_METHOD(Load_FileNotExist_ReturnsZero)
    {
        std::string path = TempPath("notexist");
        Cleanup(path);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
    }

    // 用例 2：空文件 → 读取返回 0
    TEST_METHOD(Load_EmptyFile_ReturnsZero)
    {
        std::string path = TempPath("empty");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 3：正常数字 → 读回原值
    TEST_METHOD(Load_ValidNumber_ReturnsValue)
    {
        std::string path = TempPath("valid");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "123");
        fclose(fp);
        Assert::AreEqual(123, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 4：数字带首尾空白和换行 → 正常解析
    TEST_METHOD(Load_NumberWithWhitespace_ReturnsValue)
    {
        std::string path = TempPath("ws");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "  42\r\n");
        fclose(fp);
        Assert::AreEqual(42, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 5：纯文本垃圾 → 返回 0
    TEST_METHOD(Load_GarbageText_ReturnsZero)
    {
        std::string path = TempPath("garbage");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "hello world");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 6：负数 → 返回 0
    TEST_METHOD(Load_NegativeNumber_ReturnsZero)
    {
        std::string path = TempPath("neg");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "-7");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 7：超出 int 范围 → 返回 0（溢出）
    TEST_METHOD(Load_NumberTooLarge_ReturnsZero)
    {
        std::string path = TempPath("overflow");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "99999999999");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 8：数字后跟多余字符 → 返回 0（内容非法）
    TEST_METHOD(Load_TrailingGarbage_ReturnsZero)
    {
        std::string path = TempPath("trail");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "12abc");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 9：NULL 路径 → 返回 0
    TEST_METHOD(Load_NullPath_ReturnsZero)
    {
        Assert::AreEqual(0, HighScoreLoad(NULL));
    }

    // 用例 10：保存后再读取 → 往返一致
    TEST_METHOD(Save_ThenLoad_RoundTrip)
    {
        std::string path = TempPath("roundtrip");
        Cleanup(path);
        Assert::AreEqual(1, HighScoreSave(path.c_str(), 100));
        Assert::AreEqual(100, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 11：第二次保存覆盖旧值
    TEST_METHOD(Save_OverwritesOldValue)
    {
        std::string path = TempPath("overwrite");
        Cleanup(path);
        HighScoreSave(path.c_str(), 50);
        Assert::AreEqual(1, HighScoreSave(path.c_str(), 300));
        Assert::AreEqual(300, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 12：保存 0 → 读回 0（0 是合法存档值）
    TEST_METHOD(Save_Zero_Ok)
    {
        std::string path = TempPath("zero");
        Cleanup(path);
        Assert::AreEqual(1, HighScoreSave(path.c_str(), 0));
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 13：负数分数 → 保存失败，且不影响已有存档
    TEST_METHOD(Save_NegativeScore_Fails)
    {
        std::string path = TempPath("negsave");
        Cleanup(path);
        HighScoreSave(path.c_str(), 100);
        Assert::AreEqual(0, HighScoreSave(path.c_str(), -1));
        Assert::AreEqual(100, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 14：NULL 路径 → 保存失败
    TEST_METHOD(Save_NullPath_Fails)
    {
        Assert::AreEqual(0, HighScoreSave(NULL, 100));
    }

    // 用例 15：非法路径（不存在的目录）→ 保存失败
    TEST_METHOD(Save_InvalidPath_Fails)
    {
        Assert::AreEqual(0, HighScoreSave("Z:\\no_such_dir_xyz\\highscore.txt", 100));
    }

    // 用例 16：文件内容只有空白 → 返回 0
    TEST_METHOD(Load_OnlyWhitespace_ReturnsZero)
    {
        std::string path = TempPath("onlyws");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "   \r\n\t");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 17：正号前缀（+123）→ 视为合法整数，读回 123
    TEST_METHOD(Load_PlusPrefixedNumber_ReturnsValue)
    {
        std::string path = TempPath("plus");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "+123");
        fclose(fp);
        Assert::AreEqual(123, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 18：前导零（007）→ 读回 7
    TEST_METHOD(Load_LeadingZeros_ReturnsValue)
    {
        std::string path = TempPath("zeros");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "007");
        fclose(fp);
        Assert::AreEqual(7, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 19：正好 INT_MAX → 读回 INT_MAX（边界合法）
    TEST_METHOD(Load_IntMax_ReturnsValue)
    {
        std::string path = TempPath("intmax");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "2147483647");
        fclose(fp);
        Assert::AreEqual(INT_MAX, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 20：INT_MAX + 1 → 返回 0（边界溢出）
    TEST_METHOD(Load_IntMaxPlusOne_ReturnsZero)
    {
        std::string path = TempPath("intmax1");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "2147483648");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 21：文件里有两个数字（"123\n456"）→ 整文件校验失败，返回 0
    TEST_METHOD(Load_MultipleLines_TwoNumbers_ReturnsZero)
    {
        std::string path = TempPath("twolines");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        fprintf(fp, "123\n456");
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 22：文件超过 4096 字节 → 视为非法内容，返回 0
    TEST_METHOD(Load_TooLongContent_ReturnsZero)
    {
        std::string path = TempPath("toolong");
        FILE* fp = NULL;
        fopen_s(&fp, path.c_str(), "w");
        for (int i = 0; i < 5000; i++) fputc('a', fp);
        fclose(fp);
        Assert::AreEqual(0, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 23：路径是目录 → 读取返回 0
    TEST_METHOD(Load_PathIsDirectory_ReturnsZero)
    {
        std::string dir = TempDir("load");
        _rmdir(dir.c_str());       // 清掉可能残留的旧目录
        _mkdir(dir.c_str());
        Assert::AreEqual(0, HighScoreLoad(dir.c_str()));
        _rmdir(dir.c_str());
    }

    // 用例 24：INT_MAX 保存后读回 → 往返一致（边界）
    TEST_METHOD(Save_IntMax_RoundTrip)
    {
        std::string path = TempPath("intmaxsave");
        Cleanup(path);
        Assert::AreEqual(1, HighScoreSave(path.c_str(), INT_MAX));
        Assert::AreEqual(INT_MAX, HighScoreLoad(path.c_str()));
        Cleanup(path);
    }

    // 用例 25：路径是目录 → 保存失败
    TEST_METHOD(Save_PathIsDirectory_Fails)
    {
        std::string dir = TempDir("save");
        _rmdir(dir.c_str());
        _mkdir(dir.c_str());
        Assert::AreEqual(0, HighScoreSave(dir.c_str(), 100));
        _rmdir(dir.c_str());
    }

    // 用例 26：目标文件只读 → 保存失败，且旧存档值保留、仍可读取
    TEST_METHOD(Save_ReadOnlyFile_Fails)
    {
        std::string path = TempPath("readonly");
        Cleanup(path);
        HighScoreSave(path.c_str(), 100);
        _chmod(path.c_str(), _S_IREAD);                     // 设为只读
        Assert::AreEqual(0, HighScoreSave(path.c_str(), 200));
        Assert::AreEqual(100, HighScoreLoad(path.c_str())); // 只读不影响读取
        _chmod(path.c_str(), _S_IREAD | _S_IWRITE);         // 恢复可写
        Cleanup(path);
    }
};

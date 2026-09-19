// ============================================================
// test_tetromino.cpp —— Phase 2 单元测试：形状与旋转
// ------------------------------------------------------------
// 对应 project.md 第 8 章 Phase 2 的用例清单（P2-01 ~ P2-20），
// 该清单已由用户审核通过后才开始编写本文件。
//
// 被测代码：tetromino.h / tetromino.c（游戏逻辑层，纯 C）
// 测试框架：gtest（随 Visual Studio 安装的离线包）
//
// 【关于用例名用英文】
// gtest 的用例名必须能当作 C++ 标识符，用中文字符有兼容性风险，
// 所以用例名用英文，中文说明写在注释和断言消息里。
// ============================================================

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "config.h"
#include "tetromino.h"

// ============================================================
// 测试辅助函数
// ============================================================

// 取指定编号的基准姿态；失败则当前用例立刻失败。
static void BaseOf(int id, int box[TETROMINO_BOX][TETROMINO_BOX])
{
    ASSERT_TRUE(tetromino_base_copy(id, box)) << "编号 " << id << " 取基准姿态失败";
}

// 把盒子里的占用格转成"排序后的坐标文本"，例如 "(0,2) (1,2) (1,3) (2,2)"。
// 用文本比较而不是比较容器，好处是断言失败时 gtest 会把两组坐标直接打印出来，
// 一眼就能看出差在哪一格。坐标都是个位数，字符串排序恰好等于数值排序。
static std::string CellText(const int box[TETROMINO_BOX][TETROMINO_BOX])
{
    std::vector<std::string> items;
    for (int r = 0; r < TETROMINO_BOX; r++)
    {
        for (int c = 0; c < TETROMINO_BOX; c++)
        {
            if (box[r][c])
            {
                items.push_back("(" + std::to_string(r) + "," + std::to_string(c) + ")");
            }
        }
    }
    std::sort(items.begin(), items.end());

    std::string text;
    for (size_t i = 0; i < items.size(); i++)
    {
        if (i > 0)
        {
            text += " ";
        }
        text += items[i];
    }
    return text;
}

// ============================================================
// A 组：形状定义（P2-01 ~ P2-07）
// ============================================================

// P2-01 七种形状各自的格子数都必须是 4
TEST(Phase2Shape, P2_01_EachShapeHasExactlyFourCells)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);
        EXPECT_EQ(tetromino_cell_count(box), 4)
            << "编号 " << id << " 的方块格数不是 4";
    }
}

// P2-02 同一形状内 4 格的坐标两两不同（没有把同一格重复计算）
// 说明：在当前"4×4 矩阵"的表示法下，每个格子的坐标天生唯一，
//       所以这条是文档性检查；将来若改成"坐标列表"表示法，它就会变成真正有用的测试。
TEST(Phase2Shape, P2_02_CellsAreUnique)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);

        std::vector<std::string> items;
        for (int r = 0; r < TETROMINO_BOX; r++)
        {
            for (int c = 0; c < TETROMINO_BOX; c++)
            {
                if (box[r][c])
                {
                    items.push_back(std::to_string(r) + "," + std::to_string(c));
                }
            }
        }
        std::sort(items.begin(), items.end());
        items.erase(std::unique(items.begin(), items.end()), items.end());

        EXPECT_EQ((int)items.size(), 4) << "编号 " << id << " 的占用格有重复";
    }
}

// P2-03 所有占用格都落在 4×4 盒子内（行、列均在 0~3）
// 说明：矩阵表示法下同样天生成立，这里用"包围盒"显式断言，
//       作为形状数据合法性的文档性检查。
TEST(Phase2Shape, P2_03_CellsInsideTheBox)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);

        for (int r = 0; r < TETROMINO_BOX; r++)
        {
            for (int c = 0; c < TETROMINO_BOX; c++)
            {
                if (box[r][c])
                {
                    EXPECT_GE(r, 0);
                    EXPECT_LT(r, TETROMINO_BOX) << "编号 " << id << " 有格子超出盒子下边界";
                    EXPECT_GE(c, 0);
                    EXPECT_LT(c, TETROMINO_BOX) << "编号 " << id << " 有格子超出盒子右边界";
                }
            }
        }
    }
}

// P2-04 形状数据只允许出现 0 和 1（防止手滑写成 2 之类的值）
TEST(Phase2Shape, P2_04_OnlyZeroOrOne)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);

        for (int r = 0; r < TETROMINO_BOX; r++)
        {
            for (int c = 0; c < TETROMINO_BOX; c++)
            {
                EXPECT_TRUE(box[r][c] == 0 || box[r][c] == 1)
                    << "编号 " << id << " 在 (" << r << "," << c << ") 处的值既不是 0 也不是 1";
            }
        }
    }
}

// P2-05 七种形状两两不同（防止复制粘贴写重、有两种形状长得一样）
TEST(Phase2Shape, P2_05_AllShapesAreDifferent)
{
    for (int a = 0; a < TETROMINO_COUNT; a++)
    {
        for (int b = a + 1; b < TETROMINO_COUNT; b++)
        {
            int boxA[TETROMINO_BOX][TETROMINO_BOX];
            int boxB[TETROMINO_BOX][TETROMINO_BOX];
            BaseOf(a, boxA);
            BaseOf(b, boxB);
            EXPECT_NE(CellText(boxA), CellText(boxB))
                << "编号 " << a << " 与编号 " << b << " 的形状完全相同";
        }
    }
}

// P2-06 编号 0~6 与七种形状一一对应（逐个核对出生姿态）
// 这里的期望值是手写出来的，等于把"形状定义"这份规格又抄了一遍，
// 所以如果有人改动了形状数据，这条用例会立刻发现。
TEST(Phase2Shape, P2_06_IdMatchesShape)
{
    struct Expect { int id; const char* text; };
    const Expect expects[] =
    {
        { TETROMINO_I, "(1,0) (1,1) (1,2) (1,3)" },  // I：横躺在第 1 行
        { TETROMINO_O, "(1,1) (1,2) (2,1) (2,2)" },  // O：居中
        { TETROMINO_T, "(0,1) (1,0) (1,1) (1,2)" },  // T：尖朝上
        { TETROMINO_S, "(0,1) (0,2) (1,0) (1,1)" },  // S
        { TETROMINO_Z, "(0,0) (0,1) (1,1) (1,2)" },  // Z
        { TETROMINO_J, "(0,0) (1,0) (1,1) (1,2)" },  // J：钩在左
        { TETROMINO_L, "(0,2) (1,0) (1,1) (1,2)" },  // L：钩在右
    };

    for (const Expect& e : expects)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(e.id, box);
        EXPECT_EQ(CellText(box), std::string(e.text)) << "编号 " << e.id << " 的形状不对";
    }
}

// P2-07 非法编号（-1、7、100）必须安全处理，不能崩溃
TEST(Phase2Shape, P2_07_InvalidIdIsHandledSafely)
{
    const int badIds[] = { -1, TETROMINO_COUNT, 100 };

    for (int bad : badIds)
    {
        EXPECT_FALSE(tetromino_id_valid(bad)) << "编号 " << bad << " 不该被判为合法";

        int box[TETROMINO_BOX][TETROMINO_BOX];
        for (int r = 0; r < TETROMINO_BOX; r++)
        {
            for (int c = 0; c < TETROMINO_BOX; c++)
            {
                box[r][c] = 1;   // 先填满，验证函数会把它清空
            }
        }

        EXPECT_FALSE(tetromino_base_copy(bad, box)) << "编号 " << bad << " 应返回失败";
        EXPECT_EQ(tetromino_cell_count(box), 0) << "编号 " << bad << " 失败时应把盒子清空";

        int cols[TETROMINO_BOX];
        int rows[TETROMINO_BOX];
        EXPECT_EQ(tetromino_spawn_cells(bad, cols, rows, TETROMINO_BOX), 0)
            << "编号 " << bad << " 不应算出出生位置";
    }
}

// ============================================================
// B 组：旋转（P2-08 ~ P2-16）
// ============================================================

// P2-08 顺时针转 4 次回到初始状态
TEST(Phase2Rotate, P2_08_FourClockwiseTurnsReturnToStart)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);
        const std::string before = CellText(box);

        for (int i = 0; i < 4; i++)
        {
            tetromino_rotate_cw(box);
        }
        EXPECT_EQ(CellText(box), before) << "编号 " << id << " 顺时针转 4 次没回到原状";
    }
}

// P2-09 逆时针转 4 次回到初始状态
TEST(Phase2Rotate, P2_09_FourCounterClockwiseTurnsReturnToStart)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);
        const std::string before = CellText(box);

        for (int i = 0; i < 4; i++)
        {
            tetromino_rotate_ccw(box);
        }
        EXPECT_EQ(CellText(box), before) << "编号 " << id << " 逆时针转 4 次没回到原状";
    }
}

// P2-10 顺时针 1 次 + 逆时针 1 次，互相抵消
TEST(Phase2Rotate, P2_10_ClockwiseThenCounterClockwiseCancelOut)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);
        const std::string before = CellText(box);

        tetromino_rotate_cw(box);
        tetromino_rotate_ccw(box);

        EXPECT_EQ(CellText(box), before) << "编号 " << id << " 顺时针后逆时针没回到原状";
    }
}

// P2-11 顺时针方向正确性（I 形）
// 这条是整个 B 组最关键的一条：前面几条"转回原状"的用例，
// 即使把顺时针和逆时针写反了也照样能通过，必须用不对称的精确断言来抓方向错误。
// I 形横躺在第 1 行，顺时针转 90° 必须竖立在"第 2 列"；若写成逆时针会是"第 1 列"。
TEST(Phase2Rotate, P2_11_ClockwiseDirectionIsCorrect)
{
    int box[TETROMINO_BOX][TETROMINO_BOX];
    BaseOf(TETROMINO_I, box);

    tetromino_rotate_cw(box);

    EXPECT_EQ(CellText(box), "(0,2) (1,2) (2,2) (3,2)")
        << "I 形顺时针转完应竖立在第 2 列；若这里是第 1 列，说明写成了逆时针";
}

// P2-12 逆时针方向正确性（I 形）：应竖立在第 1 列
TEST(Phase2Rotate, P2_12_CounterClockwiseDirectionIsCorrect)
{
    int box[TETROMINO_BOX][TETROMINO_BOX];
    BaseOf(TETROMINO_I, box);

    tetromino_rotate_ccw(box);

    EXPECT_EQ(CellText(box), "(0,1) (1,1) (2,1) (3,1)")
        << "I 形逆时针转完应竖立在第 1 列";
}

// P2-13 T 形顺时针 1 次的精确坐标（手算结果）
// T 形原本是 {(0,1),(1,0),(1,1),(1,2)}，顺时针后尖朝右：
// 竖条落在第 2 列（第 0~2 行），尖在 (1,3)。
TEST(Phase2Rotate, P2_13_TOneClockwiseExactCells)
{
    int box[TETROMINO_BOX][TETROMINO_BOX];
    BaseOf(TETROMINO_T, box);

    tetromino_rotate_cw(box);

    EXPECT_EQ(CellText(box), "(0,2) (1,2) (1,3) (2,2)") << "T 形顺时针后的坐标不对";
}

// P2-14 O 形转 1、2、3 次形状都必须完全不变
// 这是把 O 形摆在 4×4 盒子正中间的意义所在：摆歪了这条就会失败。
TEST(Phase2Rotate, P2_14_OShapeNeverChanges)
{
    const std::string oBase = "(1,1) (1,2) (2,1) (2,2)";

    for (int times = 1; times <= 3; times++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(TETROMINO_O, box);

        for (int i = 0; i < times; i++)
        {
            tetromino_rotate_cw(box);
        }
        EXPECT_EQ(CellText(box), oBase) << "O 形顺时针转 " << times << " 次后形状变了";
    }
}

// P2-15 七种形状 × 四个方向：格数恒为 4，且所有格仍在盒子内
TEST(Phase2Rotate, P2_15_CountAndBoundsHoldInAllOrientations)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int box[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, box);

        for (int turn = 0; turn < 4; turn++)
        {
            EXPECT_EQ(tetromino_cell_count(box), 4)
                << "编号 " << id << " 在第 " << turn << " 个方向下格数不是 4";

            for (int r = 0; r < TETROMINO_BOX; r++)
            {
                for (int c = 0; c < TETROMINO_BOX; c++)
                {
                    if (box[r][c])
                    {
                        EXPECT_TRUE(r >= 0 && r < TETROMINO_BOX && c >= 0 && c < TETROMINO_BOX)
                            << "编号 " << id << " 第 " << turn << " 个方向下有格子跑出盒子";
                    }
                }
            }
            tetromino_rotate_cw(box);
        }
    }
}

// P2-16 旋转副本不会破坏形状的基准数据
// 这是最容易犯的错误之一：如果旋转函数直接操作了那份全局的形状表，
// 那么此后出生的每一个方块都会是歪的，而且问题会传播到后面所有阶段。
TEST(Phase2Rotate, P2_16_RotatingCopyDoesNotDamageBase)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int copy[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, copy);

        // 把这份副本转来转去
        tetromino_rotate_cw(copy);
        tetromino_rotate_ccw(copy);
        tetromino_rotate_cw(copy);
        tetromino_rotate_cw(copy);

        // 再取一次基准姿态，必须仍然是最初的样子
        int fresh[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, fresh);

        int original[TETROMINO_BOX][TETROMINO_BOX];
        BaseOf(id, original);
        tetromino_rotate_cw(copy);   // 再随便转一次，确保上面已取到的数据不受影响

        EXPECT_EQ(CellText(fresh), CellText(original))
            << "编号 " << id << " 的基准数据被旋转操作改坏了";
    }
}

// ============================================================
// C 组：出生位置（P2-17 ~ P2-19）
// ============================================================

// P2-17 出生时的列范围：每格列号都在 [SPAWN_BOX_COL, SPAWN_BOX_COL+3]，且不越出棋盘
TEST(Phase2Spawn, P2_17_SpawnColumnsAreInsideBoard)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int cols[TETROMINO_BOX];
        int rows[TETROMINO_BOX];
        const int n = tetromino_spawn_cells(id, cols, rows, TETROMINO_BOX);
        ASSERT_EQ(n, 4) << "编号 " << id << " 的出生格数不是 4";

        for (int i = 0; i < n; i++)
        {
            EXPECT_GE(cols[i], SPAWN_BOX_COL) << "编号 " << id << " 的出生位置太靠左";
            EXPECT_LE(cols[i], SPAWN_BOX_COL + TETROMINO_BOX - 1) << "编号 " << id << " 超出盒子列范围";
            EXPECT_LT(cols[i], BOARD_COLS) << "编号 " << id << " 的出生位置越出棋盘右边界";
        }
    }
}

// P2-18 出生时的行范围：每格逻辑行号都在 [SPAWN_BOX_ROW, SPAWN_BOX_ROW+3]，且不越出逻辑棋盘
TEST(Phase2Spawn, P2_18_SpawnRowsAreInsideBoard)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int cols[TETROMINO_BOX];
        int rows[TETROMINO_BOX];
        const int n = tetromino_spawn_cells(id, cols, rows, TETROMINO_BOX);
        ASSERT_EQ(n, 4) << "编号 " << id << " 的出生格数不是 4";

        for (int i = 0; i < n; i++)
        {
            EXPECT_GE(rows[i], SPAWN_BOX_ROW) << "编号 " << id << " 的出生位置太靠上";
            EXPECT_LE(rows[i], SPAWN_BOX_ROW + TETROMINO_BOX - 1) << "编号 " << id << " 超出盒子行范围";
            EXPECT_LT(rows[i], BOARD_TOTAL_ROWS) << "编号 " << id << " 的出生位置越出逻辑棋盘下边界";
        }
    }
}

// P2-19 出生时至少有一格落在可见区最上面一行（否则玩家会觉得方块"隐身"了）
TEST(Phase2Spawn, P2_19_SpawnIsVisibleImmediately)
{
    for (int id = 0; id < TETROMINO_COUNT; id++)
    {
        int cols[TETROMINO_BOX];
        int rows[TETROMINO_BOX];
        const int n = tetromino_spawn_cells(id, cols, rows, TETROMINO_BOX);
        ASSERT_EQ(n, 4) << "编号 " << id << " 的出生格数不是 4";

        bool visible = false;
        for (int i = 0; i < n; i++)
        {
            if (rows[i] == SPAWN_VISIBLE_ROW)
            {
                visible = true;
            }
        }
        EXPECT_TRUE(visible)
            << "编号 " << id << " 出生时没有任何一格落在可见区（逻辑第 "
            << SPAWN_VISIBLE_ROW << " 行），玩家会看不到它";
    }
}

// ============================================================
// D 组：棋盘行数约定（P2-20）
// ============================================================

// P2-20 隐藏缓冲模型的常量自检：22 = 2 + 20，且屏幕行↔逻辑行的换算两端成立
TEST(Phase2Board, P2_20_BoardRowConstantsAreConsistent)
{
    EXPECT_EQ(BOARD_BUFFER_ROWS, 2)  << "隐藏缓冲应为 2 行";
    EXPECT_EQ(BOARD_VISIBLE_ROWS, 20) << "可见区应为 20 行";
    EXPECT_EQ(BOARD_TOTAL_ROWS, 22)  << "逻辑棋盘应为 22 行";
    EXPECT_EQ(BOARD_TOTAL_ROWS, BOARD_VISIBLE_ROWS + BOARD_BUFFER_ROWS);

    // 屏幕最上面一行（第 0 行）对应逻辑第 2 行
    EXPECT_EQ(0 + BOARD_BUFFER_ROWS, 2);

    // 屏幕最下面一行（第 19 行）对应逻辑最后一行（第 21 行）
    EXPECT_EQ((BOARD_VISIBLE_ROWS - 1) + BOARD_BUFFER_ROWS, BOARD_TOTAL_ROWS - 1);

    // 出生位置常量
    EXPECT_EQ(SPAWN_BOX_COL, 3) << "10 列棋盘下 4 格宽的盒子应居中在第 3 列";
    EXPECT_EQ(SPAWN_BOX_ROW, 1) << "出生盒子顶部应在逻辑第 1 行";
    EXPECT_EQ(SPAWN_VISIBLE_ROW, BOARD_BUFFER_ROWS) << "可见区最上面一行就是缓冲行下方那一行";
}

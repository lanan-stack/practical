// ============================================================
// P1 单元测试：蛇身适配层（MSTest 原生单元测试）
// 覆盖：初始化/销毁、头插、尾删、长度、取蛇头/第 i 节、
//       坐标编解码、占用查询、移动模拟（头插+尾删）
// ============================================================
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// C 头文件需要用 extern "C" 包起来，避免 C++ 名称修饰
extern "C"
{
#include "config.h"
#include "DCList.h"
#include "snake_list.h"
}

TEST_CLASS(SnakeListTests)
{
public:
    // 用例 1：初始化的空蛇长度为 0，取蛇头/第 0 节返回无效值；销毁后指针置空
    TEST_METHOD(Init_EmptySnake_LengthZeroAndDestroyNull)
    {
        Snake s;
        SnakeInit(&s);
        Assert::AreEqual(0, SnakeLength(&s));
        Assert::AreEqual(SNAKE_INVALID_POS, SnakeHead(&s));
        Assert::AreEqual(SNAKE_INVALID_POS, SnakeAt(&s, 0));
        SnakeDestroy(&s);
        Assert::IsNull(s.nodes);
    }

    // 用例 2：依次头插三节，长度 3，蛇头是最新插入的一节，顺序正确
    TEST_METHOD(PushHead_ThreeSegments_HeadIsLastPushed)
    {
        Snake s;
        SnakeInit(&s);
        SnakePushHead(&s, SnakePosEncode(15, 14)); // 先插入：将来是尾巴
        SnakePushHead(&s, SnakePosEncode(15, 15)); // 中间
        SnakePushHead(&s, SnakePosEncode(15, 16)); // 最后插入：是蛇头
        Assert::AreEqual(3, SnakeLength(&s));
        Assert::AreEqual(SnakePosEncode(15, 16), SnakeHead(&s));
        Assert::AreEqual(SnakePosEncode(15, 15), SnakeAt(&s, 1));
        Assert::AreEqual(SnakePosEncode(15, 14), SnakeAt(&s, 2));
        SnakeDestroy(&s);
    }

    // 用例 3：坐标编码往返（含棋盘四个角边界）
    TEST_METHOD(PosEncoding_RoundTrip_IncludesBoardBoundaries)
    {
        // 左上角
        Assert::AreEqual(0, SnakePosEncode(0, 0));
        Assert::AreEqual(0, SnakePosRow(0));
        Assert::AreEqual(0, SnakePosCol(0));
        // 右下角
        int last = SnakePosEncode(SNAKE_BOARD_SIZE - 1, SNAKE_BOARD_SIZE - 1);
        Assert::AreEqual(SNAKE_BOARD_SIZE * SNAKE_BOARD_SIZE - 1, last);
        Assert::AreEqual(SNAKE_BOARD_SIZE - 1, SnakePosRow(last));
        Assert::AreEqual(SNAKE_BOARD_SIZE - 1, SnakePosCol(last));
        // 中间一格
        int mid = SnakePosEncode(15, 16);
        Assert::AreEqual(15, SnakePosRow(mid));
        Assert::AreEqual(16, SnakePosCol(mid));
    }

    // 用例 4：越界取第 i 节返回无效值（负下标、超长下标）
    TEST_METHOD(At_OutOfRange_ReturnsInvalid)
    {
        Snake s;
        SnakeInit(&s);
        SnakePushHead(&s, SnakePosEncode(5, 5));
        Assert::AreEqual(SNAKE_INVALID_POS, SnakeAt(&s, -1));
        Assert::AreEqual(SNAKE_INVALID_POS, SnakeAt(&s, 1));
        SnakeDestroy(&s);
    }

    // 用例 5：尾删去掉最后一节；删空后继续尾删不崩溃、长度保持 0
    TEST_METHOD(PopTail_RemovesLastAndEmptyPopIsSafe)
    {
        Snake s;
        SnakeInit(&s);
        SnakePushHead(&s, SnakePosEncode(1, 1));
        SnakePushHead(&s, SnakePosEncode(2, 2));
        SnakePushHead(&s, SnakePosEncode(3, 3));

        SnakePopTail(&s);
        Assert::AreEqual(2, SnakeLength(&s));
        Assert::AreEqual(SnakePosEncode(3, 3), SnakeHead(&s)); // 蛇头不变
        Assert::AreEqual(SnakePosEncode(2, 2), SnakeAt(&s, 1)); // 原尾巴(1,1)已删除，(2,2)成为新尾巴

        SnakePopTail(&s);
        SnakePopTail(&s);
        Assert::AreEqual(0, SnakeLength(&s));
        Assert::AreEqual(SNAKE_INVALID_POS, SnakeHead(&s)); // 删空后蛇头应为无效值

        SnakePopTail(&s); // 空蛇尾删：应安全无操作
        Assert::AreEqual(0, SnakeLength(&s));
        SnakeDestroy(&s);
    }

    // 用例 6：占用查询只对蛇身格子返回 1
    TEST_METHOD(Contains_ReturnsTrueOnlyForOccupiedCells)
    {
        Snake s;
        SnakeInit(&s);
        SnakePushHead(&s, SnakePosEncode(1, 1));
        SnakePushHead(&s, SnakePosEncode(2, 2));
        Assert::IsTrue(SnakeContains(&s, SnakePosEncode(2, 2)) == 1); // 蛇头
        Assert::IsTrue(SnakeContains(&s, SnakePosEncode(1, 1)) == 1); // 蛇尾
        Assert::IsTrue(SnakeContains(&s, SnakePosEncode(3, 3)) == 0); // 空格
        SnakeDestroy(&s);
    }

    // 用例 7：模拟蛇前进一格（头插新头 + 尾删），长度不变、各节坐标正确前移
    TEST_METHOD(MoveForward_HeadPushAndTailPop_KeepsLengthAndSlides)
    {
        Snake s;
        SnakeInit(&s);
        // 开局：尾(15,14) 身(15,15) 头(15,16)，头朝右
        SnakePushHead(&s, SnakePosEncode(15, 14));
        SnakePushHead(&s, SnakePosEncode(15, 15));
        SnakePushHead(&s, SnakePosEncode(15, 16));

        // 向右走一格：新头(15,17)，尾巴(15,14)消失
        SnakePushHead(&s, SnakePosEncode(15, 17));
        SnakePopTail(&s);

        Assert::AreEqual(3, SnakeLength(&s));
        Assert::AreEqual(SnakePosEncode(15, 17), SnakeHead(&s));
        Assert::AreEqual(SnakePosEncode(15, 16), SnakeAt(&s, 1));
        Assert::AreEqual(SnakePosEncode(15, 15), SnakeAt(&s, 2));
        Assert::IsTrue(SnakeContains(&s, SnakePosEncode(15, 14)) == 0); // 旧尾巴已释放
        SnakeDestroy(&s);
    }

    // 用例 8：空蛇的占用查询应返回 0（不崩溃）
    TEST_METHOD(Contains_OnEmptySnake_ReturnsFalse)
    {
        Snake s;
        SnakeInit(&s);
        Assert::IsTrue(SnakeContains(&s, SnakePosEncode(0, 0)) == 0);
        Assert::IsTrue(SnakeContains(&s, SnakePosEncode(15, 16)) == 0);
        SnakeDestroy(&s);
    }

    // 用例 9：重复销毁应安全无操作，指针保持空
    TEST_METHOD(Destroy_Twice_IsSafeAndStaysNull)
    {
        Snake s;
        SnakeInit(&s);
        SnakePushHead(&s, SnakePosEncode(5, 5));
        SnakeDestroy(&s);
        Assert::IsNull(s.nodes);

        SnakeDestroy(&s); // 第二次销毁：适配层应判断空指针并安全跳过
        Assert::IsNull(s.nodes);
    }
};

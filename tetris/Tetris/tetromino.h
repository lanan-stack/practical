// ============================================================
// tetromino.h —— 七种方块的形状与旋转（逻辑层）
// ------------------------------------------------------------
// 所属层次：游戏逻辑层（纯 C，**不依赖 raylib**，可独立单元测试）
//
// 职责：
//   · 定义七种方块（I/O/T/S/Z/J/L）的形状数据；
//   · 提供顺时针 / 逆时针旋转；
//   · 提供出生位置的计算。
//
// 边界：本模块只管"形状长什么样、怎么转"，不涉及棋盘、碰撞、消行、画图。
// ============================================================
#ifndef TETROMINO_H
#define TETROMINO_H

#include <stdbool.h>
#include "config.h"

// 这门语言里 C 和 C++ 的函数命名规则不同（C++ 会做名称改编）。
// 单元测试是 C++ 写的，会包含本头文件；加上 extern "C" 让两边命名一致，
// 否则链接时会报"无法解析的外部符号"。
#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// 形状编号
// ------------------------------------------------------------
// 顺序是固定的，**颜色表按同样下标对应**（ui_config.h 里的 BLOCK_COLORS）：
//   0=I 青、1=O 黄、2=T 紫、3=S 绿、4=Z 红、5=J 蓝、6=L 橙。
// 这些编号会被存进循环队列（袋子系统），所以 0 是合法数据 ——
// 这也是"队列空不能用 0 当哨兵值"的原因。
// ============================================================
typedef enum TetrominoId
{
    TETROMINO_I = 0,
    TETROMINO_O = 1,
    TETROMINO_T = 2,
    TETROMINO_S = 3,
    TETROMINO_Z = 4,
    TETROMINO_J = 5,
    TETROMINO_L = 6,
    TETROMINO_COUNT = 7     // 形状种类数量，也可用作"编号是否越界"的界限
} TetrominoId;

// ---------- 判断编号是否合法 ----------
// 参数：id —— 形状编号
// 返回：true = 是 0~6 的合法编号；false = 非法
bool tetromino_id_valid(int id);

// ---------- 清空盒子 ----------
// 把 4×4 矩阵全部置 0。
void tetromino_clear(int box[TETROMINO_BOX][TETROMINO_BOX]);

// ---------- 取基准姿态 ----------
// 参数：id  —— 形状编号
//       box —— 调用方提供的 4×4 数组（会被写入结果）
// 返回：true = 成功；false = 编号非法（此时 box 会被清空）
//
// 【用法约定】想让方块旋转时，先用本函数把基准姿态**复制**到自己的数组里，
//             再对那份副本旋转。绝不要直接改基准数据，否则后面出生的方块全会歪。
bool tetromino_base_copy(int id, int box[TETROMINO_BOX][TETROMINO_BOX]);

// ---------- 统计方块格数 ----------
// 参数：box —— 4×4 矩阵
// 返回：矩阵中值为 1 的格子数量（标准方块任何姿态下都是 4）
int tetromino_cell_count(const int box[TETROMINO_BOX][TETROMINO_BOX]);

// ---------- 旋转 90° ----------
// 参数：box —— 4×4 矩阵，**原地修改**
// 说明：顺时针对应"把矩阵整体往右转"，逆时针对应"往左转"。
//       旋转是刚体运动：格数不变，且所有格仍留在盒子内。
void tetromino_rotate_cw(int box[TETROMINO_BOX][TETROMINO_BOX]);   // 顺时针
void tetromino_rotate_ccw(int box[TETROMINO_BOX][TETROMINO_BOX]);  // 逆时针

// ---------- 计算出生时的占用格 ----------
// 参数：id       —— 形状编号
//       outCols  —— 输出：每格的列号（逻辑棋盘坐标）
//       outRows  —— 输出：每格的行号（逻辑棋盘坐标，含隐藏缓冲行）
//       capacity —— outCols / outRows 数组的容量（至少 4）
// 返回：实际写入的格子数量；编号非法或容量不足时返回 0。
int tetromino_spawn_cells(int id, int outCols[], int outRows[], int capacity);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // TETROMINO_H

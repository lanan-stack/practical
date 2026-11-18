#define _CRT_SECURE_NO_WARNINGS 1
#pragma once
//写需要的数据类型和游戏声明
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define EASY_COUNT 10//随机布置10个雷

#define ROW 9
#define COL 9

#define ROWS ROW+2
#define COLS COL+2

void initboard(char board[ROWS][COLS], int rows, int cols, char set);//初始化mine数组和show数组
void displayboard(char board[ROWS][COLS], int row, int col);//打印雷的个数的棋盘
void setmineboard(char board[ROWS][COLS], int row, int col);//打印布置雷的棋盘
void findmine(char mine[ROWS][COLS], char show[ROWS][COLS], int row, int col);//排查雷


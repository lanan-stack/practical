#define _CRT_SECURE_NO_WARNINGS 1

#include "game.h"
//写游戏的测试逻辑
void menu()
{
	printf("***************\n");
	printf("***1.开始游戏***\n");
	printf("***0.退出游戏***\n");
	printf("***************\n");
}
void game()
{
	char mine[11][11];//存放雷和非雷的信息
	char show[11][11];//存放雷的个数的信息
	initboard(mine, ROWS, COLS, '0');//初始化mine数组
	initboard(show, ROWS, COLS, '*');//初始化show数组
	displayboard(show, ROW, COL);//打印存放雷的个数的棋盘
	setmineboard(mine, ROW, COL);//存放雷和非雷的信息的棋盘
	findmine(mine, show, ROW, COL);//排查雷

}

int main()
{
	
	int input = 0;
	srand((unsigned int)time(NULL));
	do
	{
		menu();
		printf("请选择：");
		scanf("%d", &input);
		
		switch (input)
		{
		case 1:
			game();
			break;
		case 0:
			printf("游戏退出");
			break;
		default:
			printf("输入错误，请重新输入：");
			break;
		}
	} while (input);


	return 0;
}
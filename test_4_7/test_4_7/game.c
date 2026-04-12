//实现该游戏，写函数
#include "game.h"
void initboard(char board[ROWS][COLS], int rows, int cols, char set)
{
	int i = 0;
	for (i = 0; i < rows; i++)
	{
		int j = 0;
		for (j = 0; j < cols; j++)
		{
			board[i][j] = set;
		}
	}
}

void displayboard(char board[ROWS][COLS], int row, int col)//存放雷的个数的信息
{
	int i = 0;
	printf("******扫雷游戏********\n");
	for (i = 0; i <= col; i++)
	{
		printf("%d ", i);
	}
	printf("\n");
	
		for (i = 1; i <= row; i++)
		{
			printf("%d ", i);
			int j = 0;
			for (j = 1; j <= col; j++)
			{
				printf("%c ", board[i][j]);
			}
			printf("\n");
		}
}

void setmineboard(char board[ROWS][COLS], int row, int col)//布置雷
{
	int count = 0;//随机布置10个雷
	int x = 0;
	int y = 0;
	while (count < EASY_COUNT)
	{
		x = rand() % row + 1;
		y = rand() % col + 1;
		if (board[x][y] == '0')
		{
			board[x][y] = '1';
			count++;
		}
	}
}

int getminecount(char mine[ROWS][COLS], int x, int y)
{
	return (mine[x - 1][y] + mine[x - 1][y - 1] + mine[x][y - 1] + mine[x + 1][y -
		1] + mine[x + 1][y] +
		mine[x + 1][y + 1] + mine[x][y + 1] + mine[x - 1][y + 1] - 8 * '0');
}
void findmine(char mine[ROWS][COLS], char show[ROWS][COLS], int row, int col)
{
	int x = 0;
	int y = 0;
	int win = 0;
	int count = 0;
	while (win < row * col - EASY_COUNT)
	{
		printf("请输入要排查的坐标：\n");
		scanf("%d %d", &x, &y);
		if (x >= 1 && x <= row && y >= 1 && y <= col)
		{
			if (mine[x][y] == '1')
			{
				printf("很遗憾，你被炸死了！\n");
				displayboard(mine, ROW, COL);
				break;
			}
			else
			{
				//显示该位置周围有几个雷
				int count = getminecount(mine, ROW, COL);
				show[x][y] = count + '0';
				displayboard(show, ROW, COL);
				win++;
			}
		}
		else
		{
			printf("输入错误，请重新输入：\n");
		}
	}
	if (win == row * col - EASY_COUNT)
	{
		printf("恭喜你，游戏获得胜利！\n");
		displayboard(mine, ROW, COL);
	}
}


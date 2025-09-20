#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

int gameOver = 0;
int win = 0;
int firstClick = 1;

int speed = 12;
int revealed[16][16] = {0};
int flagged[16][16] = {0};
// 初始化地图
int** map = NULL;

// 光标跳转
void cursor(int x, int y) {
    COORD pos;
    pos.X = x - 1;
    pos.Y = y - 1;
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(handle, pos);
}

// 字符颜色
void color(int key) {
    if (key == -1) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }
    else if (key >= 0 && key < 16) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), key);
    }
}

// 地图生成器
int** generator(int safeX, int safeY) {
    srand((unsigned int)time(NULL));
    
    // 初始化地图
    int** result = (int**)malloc(16 * sizeof(int*));
    for (int i = 0; i < 16; i ++) {
        result[i] = (int*)calloc(16, sizeof(int));
    }
    
    // 生成40个点
    int count = 1;
    while (count <= 40) {
        int x = rand() % 16;
        int y = rand() % 16;
        if (result[x][y] == 0 && !(x == safeX && y == safeY)) {
            result[x][y] = 1;
            count ++;
        }
    }
    
    return result;
}

// 逐字输出
void type(const char *content) {
    // 内容输出
    int length = strlen(content);
    for (int i = 0; i < length; i ++) {
        putchar(content[i]);
        fflush(stdout);
        
        // 处理停顿
        int variation = rand() % 21 - 10;
        Sleep(speed + variation);
    }
}

// 位置解析器函数
int *analysis(char *position) {
    int *result = (int*)malloc(2 * sizeof(int));
    
    // 水平坐标
    int col = position[0];
    int x;
    if (col >= 'A' && col <= 'Z') {
        x = col - 'A' + 1;
    }
    else if (col >= 'a' && col <= 'z') {
        x = col - 'a' + 1;
    }
    else {
        result[0] = 1;
        result[1] = 1;
        return result;
    }
    
    // 竖直坐标
    int length = strlen(position);
    int y;
    if (length > 1) {
        y = atoi(position + 1);
    }
    else {
        y = 1;
    }
    
    if (x < 0 || x > 16 || y < 0 || y > 16) {
        result[0] = 1;
        result[1] = 1;
    }
    else {
        result[0] = x;
        result[1] = y;
    }
    
    return result;
}

// 计算光标位置并移动
void firstPosition(int *position) {
    int xEstimate = 8;
    int yEstimate = 4;
    int x = xEstimate + 2 * position[0] - 1;
    int y = yEstimate + position[1];
    Sleep(32);
    cursor(x, y);
}

// 定位器
int *positionAnalysis(int x, int y, int arrow) {
    int *result = (int*)malloc(2 * sizeof(int));
    
    switch(arrow) {
        case 72:    // up
            y -= 1;
            break;
        case 80:    // down
            y += 1;
            break;
        case 75:    // left
            x -= 1;
            break;
        case 77:    // right
            x += 1;
            break;
    }
    
    // 边界条件
    if (x < 1) x = 1;
    if (x > 16) x = 16;
    if (y < 1) y = 1;
    if (y > 16) y = 16;
    
    result[0] = x;
    result[1] = y;
    firstPosition(result);
    return result;
}

// 数量统计
int mine(int x, int y, int** map) {
    // 检查坐标有效
    if (x < 1 || x > 16 || y < 1 || y > 16) {
        return 10;
    }
    
    if (map[x - 1][y - 1] == 1) {
        return 9;
    }
    else {
        int number = 0;
        int start_i = (x - 1) < 1 ? 1 : x - 1;
        int end_i = (x + 1) > 16 ? 16 : x + 1;
        int start_j = (y - 1) < 1 ? 1 : y - 1;
        int end_j = (y + 1) > 16 ? 16 : y + 1;
        for (int i = start_i; i <= end_i; i ++) {
            for (int j = start_j; j <= end_j; j ++) {
                if (map[i - 1][j - 1] == 1) {
                    number ++;
                }
            }
        }
        return number;
    }
}

// 显示
int screen(int x, int y, int mineNumber, int revealed[16][16], int flagged[16][16]) {
    if (flagged[x-1][y-1]) {
        color(2);
        printf("F");
        color(-1);
        return 0;
    }
    else if (!revealed[x-1][y-1]) {
        printf(" ");
        return 0;
    }
    else if (mineNumber == 9) {
        color(4);
        printf("*");
        color(-1);
        Sleep(32);
        return 1;
    }
    else if (mineNumber == 0) {
        printf("0");
    }
    else {
        color(9);
        printf("%d", mineNumber);
        color(-1);
    }
    return 0;
}

// 递归
void revealArea(int x, int y, int** map, int revealed[16][16], int flagged[16][16]) {
    // 检查坐标是否揭示
    if (x < 1 || x > 16 || y < 1 || y > 16 || revealed[x-1][y-1] || flagged[x-1][y-1]) {
        return;
    }
    
    // 揭示当前格子
    revealed[x-1][y-1] = 1;
    int mineNumber = mine(x, y, map);
    
    // 更新显示
    firstPosition((int[2]){x, y});
    screen(x, y, mineNumber, revealed, flagged);
    
    // 揭示周围格子
    if (mineNumber == 0) {
        revealArea(x-1, y-1, map, revealed, flagged);
        revealArea(x-1, y, map, revealed, flagged);
        revealArea(x-1, y+1, map, revealed, flagged);
        revealArea(x, y-1, map, revealed, flagged);
        revealArea(x, y+1, map, revealed, flagged);
        revealArea(x+1, y-1, map, revealed, flagged);
        revealArea(x+1, y, map, revealed, flagged);
        revealArea(x+1, y+1, map, revealed, flagged);
    }
}

// 检查胜利条件
int checkWin(int** map, int revealed[16][16]) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (map[i][j] == 0 && !revealed[i][j]) {
                return 0;
            }
        }
    }
    return 1;
}

// 显示所有地雷
void showAllMines(int** map, int revealed[16][16], int flagged[16][16]) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (map[i][j] == 1 && !flagged[i][j]) {
                // 计算屏幕坐标
                int xEstimate = 8;
                int yEstimate = 4;
                int screenX = xEstimate + 2 * (i+1) - 1;
                int screenY = yEstimate + (j+1);
                cursor(screenX, screenY);
                color(4);
                printf("*");
                color(-1);
                Sleep(32);
            }
        }
    }
}

void Enter(int _x,int _y)
{
	if(revealed[_x-1][_y-1])
	{
		int countflagged=0;
		int countmine=0;
		for(int i=-1;i<2;i++)
		{
			for(int j=-1;j<2;j++)
			{
				if((i||j)&&_x+i>0&&_x+i<17&&_y+j>0&&_y+j<17)
				{
					if(flagged[_x+i-1][_y+j-1])countflagged++;
					if(map[_x+i-1][_y+j-1])countmine++;
				}
			}
		}
		if(countflagged>=countmine)
		{
			for(int i=-1;i<2;i++)
			{
				for(int j=-1;j<2;j++)
				{
					int x=_x+i,y=_y+j;
					if(x<1||x>16||y<1||y>16)continue;
					if (!flagged[x-1][y-1]&&!revealed[x-1][y-1]) {
						int mineNumber = mine(x, y, map);
						if (mineNumber == 9) {
							// 游戏结束
							revealed[x-1][y-1] = 1;
							firstPosition((int[2]){x, y});
							screen(x, y, mineNumber, revealed, flagged);
							gameOver = 1;
							
							showAllMines(map, revealed, flagged);
							return;
						}
						else {
							revealArea(x, y, map, revealed, flagged);
							
							// 检查胜利
							if (checkWin(map, revealed)) {
								win = 1;
								gameOver = 1;
							}
							
							if (!gameOver) {
								firstPosition((int[2]){x, y});
							}
						}
					}
				}
			}
		}
	}
	else
	{
		int x=_x,y=_y;
		if (!flagged[x-1][y-1]) {
			int mineNumber = mine(x, y, map);
			if (mineNumber == 9) {
				// 游戏结束
				revealed[x-1][y-1] = 1;
				firstPosition((int[2]){x, y});
				screen(x, y, mineNumber, revealed, flagged);
				gameOver = 1;
				
				showAllMines(map, revealed, flagged);
			}
			else {
				revealArea(x, y, map, revealed, flagged);
				
				// 检查胜利
				if (checkWin(map, revealed)) {
					win = 1;
					gameOver = 1;
				}
				
				if (!gameOver) {
					firstPosition((int[2]){x, y});
				}
			}
		}
	}
}

int main() {
    // 初始化
    system("cls");
    system("title Minesweeper");
    int x, y;
    int count;
    
    // 头部
    const char *head = 
    "================Minesweeper=================\n\n";
    const char *alpha = "       >A B C D E F G H I J K L M N O P<    \n";
    const char *col = "^ ";
    const char *ln = "> ";
    type(head);
    Sleep(128);
    type(alpha);
	count=8;
	while(count--)type(" ");
	count=16;
	while(count--)type(col);
    printf("\n");
    
    
    for (x = 0; x < 16; x ++) {
        count = 1;
        if (x < 9) {
            while (count < 6) {
                printf(" ");
                Sleep(speed);
                count ++;
            }
        }
        else {
            while (count < 5) {
                printf(" ");
                Sleep(speed);
                count ++;
            }
        }
        //count = 1;
        printf ("%d", x + 1);
        Sleep(speed);
        type(ln);
        
        printf("\n");
    }
    
    // 初始化光标位置
    char enter[8];
    int *position;
    cursor(1, 22);
    Sleep(128);
    type("Choose a point(e.g. i4)> ");
    scanf("%s", enter);
    position = analysis(enter);
    x = position[0];
    y = position[1];
    
    // 生成地图
    map = generator(x-1, y-1);
    firstClick = 0;
    
    // 揭示首次点击的位置
    revealArea(x, y, map, revealed, flagged);
    
    // 检查胜利条件
    if (checkWin(map, revealed)) {
        win = 1;
        gameOver = 1;
    }
    
    firstPosition((int[2]){x, y});
    
    while (!gameOver) {
        firstPosition((int[2]){x, y});
        
        int arrow = getch();
        
        if (arrow == 13) { // enter
            Enter(x,y);
        }
        else if (arrow == 32) { // space
            if (!revealed[x-1][y-1]) {
                flagged[x-1][y-1] = !flagged[x-1][y-1];
                firstPosition((int[2]){x, y});
                int mineNumber = mine(x, y, map);
                screen(x, y, mineNumber, revealed, flagged);
                
                firstPosition((int[2]){x, y});
            }
        }
        else {
            // 移动光标
            position = positionAnalysis(x, y, arrow);
            x = position[0];
            y = position[1];
        }
    }
    
    // 结束游戏
    cursor(1, 24);
    Sleep(1000);
    if (win) {
        type("You win!\n");
    }
    else {
        type("Game over.\n");
    }
    cursor(1, 26);
    Sleep(1000);
    system("pause");
    
    if (map != NULL) {
        for (int i = 0; i < 16; i ++) {
            free(map[i]);
        }
        free(map);
    }
    free(position);
    
    return 0;
}

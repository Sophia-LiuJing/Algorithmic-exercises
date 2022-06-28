#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
// 告诉连接器与WS2_32库连接
#pragma comment(lib,"WS2_32.lib")
#define BUF_SIZE 256
#define NAME_SIZE 30
#define PASSWORD_SIZE 50
#define INPUT_ROWS 2

DWORD WINAPI send_msg(LPVOID lpParam);
DWORD WINAPI recv_msg(LPVOID lpParam);
HANDLE drawConsole();
void error_handling(const char* msg);
void dramPoint(COORD pos, HANDLE hout, int sign, int strLen);
int login();

SOCKET sock;
HANDLE hMutex; //互斥锁
char name[NAME_SIZE];
char password[PASSWORD_SIZE];
char msg[BUF_SIZE];
char line1[111]; // 分割线
char line2[111]; //一行空白字符

// 控制台界面绘制相关变量
DWORD num, width, height;
int output_bottom, rows, length;
COORD pos;
CHAR_INFO scroll_fill_char;
CONSOLE_SCREEN_BUFFER_INFO csbi;
SMALL_RECT message_region;
HANDLE hout;

int main()
{
	HANDLE hThread[2];
	DWORD dwThreadId;
	// 初始化WS2_32.dll
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	WSAStartup(sockVersion, &wsaData);

	// 创建套节字
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		error_handling("Failed socket()");

	// 填写远程地址信息
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(8888);
	// 如果你的计算机没有联网，直接使用本地地址127.0.0.1
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (connect(sock, (sockaddr*)&servAddr, sizeof(servAddr)) == -1)
		error_handling("Failed connect()");
	//printf("connect success\n");

	//登录
	int loginFeedback=login();
	if(loginFeedback!=1){
        printf("%d\n",loginFeedback);
        return 0;
	}
    hout = drawConsole();

	hThread[0] = CreateThread(
		NULL,		// 默认安全属性
		NULL,		// 默认堆栈大小
		send_msg,	// 线程入口地址（执行线程的函数）
		&sock,		// 传给函数的参数
		0,		// 指定线程立即运行
		&dwThreadId);	// 返回线程的ID号
	hThread[1] = CreateThread(
		NULL,		// 默认安全属性
		NULL,		// 默认堆栈大小
		recv_msg,	// 线程入口地址（执行线程的函数）
		&sock,		// 传给函数的参数
		0,		// 指定线程立即运行
		&dwThreadId);	// 返回线程的ID号

	// 等待线程运行结束
	WaitForMultipleObjects(2, hThread, true, INFINITE);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	printf(" Thread Over,Enter anything to over.\n");
	getchar();
	// 关闭套节字
	closesocket(sock);
	// 释放WS2_32库
	WSACleanup();
	return 0;
}

DWORD WINAPI send_msg(LPVOID lpParam)
{
	int sock = *((int*)lpParam);
	int tempSign = 0;
	char name_msg[NAME_SIZE + BUF_SIZE];
	while (1)
	{
		fgets(msg, BUF_SIZE, stdin);
		//去除进入的空格
		if(tempSign == 0) {
			tempSign = 1;
			continue;
		}
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			closesocket(sock);
			exit(0);
		}
		sprintf(name_msg, "[%s]:%s", name, msg);
		dramPoint(pos, hout, 1, 0);
		FillConsoleOutputCharacter(hout, ' ', sizeof(name_msg), pos, &num);
		dramPoint(pos, hout, 1, 0);
		int nRecv = send(sock, name_msg, strlen(name_msg), 0);
	}
	return NULL;
}

DWORD WINAPI recv_msg(LPVOID lpParam)
{
	int sock = *((int*)lpParam);
	char name_msg[NAME_SIZE + BUF_SIZE];
	int str_len;
	while (1)
	{
		str_len = recv(sock, name_msg, NAME_SIZE + BUF_SIZE - 1, 0);
		if (str_len == -1)
			return -1;
		name_msg[str_len] = 0;
		dramPoint(pos, hout, 0, 100);
		fputs(name_msg, stdout);
		dramPoint(pos, hout, 1, 0);
	}
	return NULL;
}

void error_handling(const char* msg)
{
	printf("%s\n", msg);
	WSACleanup();
	exit(1);
}

int login(){
    //配置聊天室的登录窗口
    //system("mode con lines=10 cols=40");
    printf("\n\n                    欢迎使用多人聊天室\n\n");
    printf("                   昵称：");
    scanf("%s",name);
    getchar();
    printf("                   密码：");
    scanf("%s",password);
    getchar();
    //把自己的昵称，发送给服务器
    send(sock, name,strlen(name), 0);
    send(sock,password,strlen(password),0);

    recv(sock, msg, sizeof(msg),0);
    if(strcmp(msg ,"0")==0){
        system("cls");
        printf("用户不存在\n");
        return 0;
    }else if(strcmp(msg, "2")==0){
        system("cls");
        printf("密码错误\n");
        return 2;
    }else return 1;
}

HANDLE drawConsole() {
    system("cls");
	scroll_fill_char.Char.AsciiChar = ' ';
    scroll_fill_char.Attributes = FOREGROUND_GREEN;

    HANDLE hout = GetStdHandle( STD_OUTPUT_HANDLE );

    GetConsoleScreenBufferInfo(hout, &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    output_bottom = height - INPUT_ROWS - 2;

    message_region.Left = message_region.Top = 0;
    message_region.Right = width - 1;
    message_region.Bottom = output_bottom;

    //画分隔线
    pos.X = 0;
    pos.Y = output_bottom + 1;
    FillConsoleOutputAttribute(hout, FOREGROUND_BLUE, width, pos, &num);
    FillConsoleOutputCharacter(hout, '=', width, pos, &num);

    return hout;
}

void dramPoint(COORD pos, HANDLE hout, int sign, int strLen) {
	if(sign == 1) {
		pos.X = 0;
		pos.Y = height - INPUT_ROWS;
		SetConsoleCursorPosition(hout, pos);
	} else {
		// 消息区域进行滚动
		rows = (strLen + width - 1) / width;
		pos.X = 0;
        pos.Y = -rows;
        ScrollConsoleScreenBuffer(hout, &message_region, &message_region, pos, &scroll_fill_char);

		// 设置光标位置
		pos.X = 0;
        pos.Y = height - INPUT_ROWS - rows - 1;
        FillConsoleOutputCharacter(hout, ' ', width*rows, pos, &num);
		SetConsoleCursorPosition(hout, pos);
	}
}

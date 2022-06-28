#include <winsock2.h>
#include <stdio.h>
#include <windows.h>

// 告诉连接器与WS2_32库连接
#pragma comment(lib,"WS2_32.lib")
#define BUF_SIZE 256
#define NAME_SIZE 30
#define PASSWORD_SIZE 50

DWORD WINAPI send_msg(LPVOID lpParam);
DWORD WINAPI recv_msg(LPVOID lpParam);
void error_handling(const char* msg);
int login();
void uiInit();
void gotoxy(int col,int row);

SOCKET sock;
HANDLE hMutex; //互斥锁
static POINT pos={0,0};
char name[NAME_SIZE];
char password[PASSWORD_SIZE];
char msg[BUF_SIZE];
char line1[111]; // 分割线
char line2[111]; //一行空白字符

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
	printf("connect success\n");

	//登录
	int loginFeedback=login();
	if(loginFeedback!=1){
        printf("%d\n",loginFeedback);
        return 0;
	}
    uiInit();

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
	char name_msg[NAME_SIZE + BUF_SIZE];
	while (1)
	{
		fgets(msg, BUF_SIZE, stdin);
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			closesocket(sock);
			exit(0);
		}
		sprintf(name_msg, "[%s]: %s", name, msg);
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
		fputs(name_msg, stdout);
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
    system("mode con lines=10 cols=40");
    printf("\n\n     欢迎使用多人聊天室\n\n");
    printf("       昵称：");
    scanf("%s",name);
    getchar();
    printf("       密码：");
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

void gotoxy(int col,int row){

    HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE);//获取标准控制台的句柄
    COORD postem={col,row};
    //设置指定控制台的光标位置
    SetConsoleCursorPosition(handle, postem);//配置哪个控制台的光标，把光标移动到哪个位置

}

void uiInit(){ //聊天界面初始化
    system("mode con lines=36 cols=110");
    system("cls"); //清除屏幕
    for(int i=0;i<110;i++){
        line1[i]='-';
    }
    line1[110]=0;
    gotoxy(0,33);//移动光标位置
    printf("%s\n\n",line1);
}

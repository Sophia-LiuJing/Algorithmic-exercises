#include <winsock2.h>	// 为了使用Winsock API函数
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#define MAX_CLNT 256
#define BUF_SIZE 100
#define NAME_SIZE 30
#define PASSWORD_SIZE 50
#define msgNumber 10
// 告诉连接器与WS2_32库连接
#pragma comment(lib,"WS2_32.lib")

void error_handling(const char* msg);		/*错误处理函数*/
DWORD WINAPI ThreadProc(LPVOID lpParam);	/*线程执行函数*/
void send_msg(char* msg, int len);			/*消息发送函数*/
void loginVerification(char* name,char* password); /*登录验证*/
void readSaveMsg();                                 /*读取缓存消息*/
void saveNewMsg();                                  /*保存最新数据*/
void updateMsg(char *msg);                         /*更新缓存数据*/

HANDLE g_hEvent;			/*事件内核对象*/
int clnt_cnt = 0;			//统计套接字
int clnt_socks[MAX_CLNT];	//管理套接字
HANDLE hThread[MAX_CLNT];	//管理线程
char userName[NAME_SIZE];
char password[PASSWORD_SIZE];
char loginFeedback[2];
char saveMsg[msgNumber][2048];//保存最新的十条消息

int main()
{
	// 初始化WS2_32.dll
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	WSAStartup(sockVersion, &wsaData);	//请求了一个2.2版本的socket

	// 创建一个自动重置的（auto-reset events），受信的（signaled）事件内核对象
	g_hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

	// 创建套节字
	SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serv_sock == INVALID_SOCKET)
		error_handling("Failed socket()");

	// 填充sockaddr_in结构
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);				//8888端口
	sin.sin_addr.S_un.S_addr = INADDR_ANY;	//本地地址
	//sin.sin_addr.S_un.S_addr = inet_addr("169.254.211.52");

	// 绑定这个套节字到一个本地地址
	if (bind(serv_sock, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		error_handling("Failed bind()");

	// 进入监听模式
	if (listen(serv_sock, 256) == SOCKET_ERROR)		//最大连接数为2
		error_handling("Failed listen()");
	printf("Start listen:\n");
	// 循环接受客户的连接请求
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	DWORD dwThreadId;	/*线程ID*/
	SOCKET clnt_sock;
	//char szText[] = "hello！\n";
	while (TRUE)
	{
		printf("等待新连接\n");
		// 接受一个新连接
		clnt_sock = accept(serv_sock, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if (clnt_sock == INVALID_SOCKET)
		{
			printf("Failed accept()");
			continue;
		}

		/*等待内核事件对象状态受信*/
		WaitForSingleObject(g_hEvent, INFINITE);
		hThread[clnt_cnt] = CreateThread(
			NULL,		// 默认安全属性
			NULL,		// 默认堆栈大小
			ThreadProc,	// 线程入口地址（执行线程的函数）
			(void*)&clnt_sock,		// 传给函数的参数
			0,		// 指定线程立即运行
			&dwThreadId);	// 返回线程的ID号
		clnt_socks[clnt_cnt++] = clnt_sock;
		SetEvent(g_hEvent);				/*设置受信*/

		printf(" 接受到一个连接：%s 执行线程ID：%d\r\n", inet_ntoa(remoteAddr.sin_addr), dwThreadId);
	}
	WaitForMultipleObjects(clnt_cnt, hThread, true, INFINITE);
	for (int i = 0; i < clnt_cnt; i++)
	{
		CloseHandle(hThread[i]);
	}
	// 关闭监听套节字
	closesocket(serv_sock);
	// 释放WS2_32库
	WSACleanup();
	return 0;
}


void error_handling(const char* msg)
{
	printf("%s\n", msg);
	WSACleanup();
	exit(1);
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	int clnt_sock = *((int*)lpParam);
	int str_len = 0, i;
	char msg[BUF_SIZE];

	//接收新连接所发送的消息
    recv(clnt_sock, userName, sizeof(userName), 0);
    recv(clnt_sock, password, sizeof(password), 0);
    printf("%s. %s\n",userName,password);
    //判断用户是否存在以及密码是否正确
    loginVerification(userName,password);
    send(clnt_sock, loginFeedback, sizeof(loginFeedback), 0);

    //读取缓存消息
    readSaveMsg();

    //用户进入聊天室提示
    char te[2048];
    strcpy(te,userName);
    strcat(te,"进入聊天室\n");
    send_msg(te,strlen(te));

	while ((str_len = recv(clnt_sock, msg, sizeof(msg), 0)) != -1)
	{
		send_msg(msg, str_len);
		updateMsg(msg);
		strcpy(saveMsg[msgNumber],msg);
        saveNewMsg();
		printf("群发送成功\n");
	}
	printf("客户端退出:%d\n", GetCurrentThreadId());
	/*等待内核事件对象状态受信*/
	WaitForSingleObject(g_hEvent, INFINITE);
	for (i = 0; i < clnt_cnt; i++)
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;
	SetEvent(g_hEvent);		/*设置受信*/
	// 关闭同客户端的连接
	closesocket(clnt_sock);
	return NULL;
}

void send_msg(char* msg, int len)
{
	int i;
	/*等待内核事件对象状态受信*/
	WaitForSingleObject(g_hEvent, INFINITE);
	for (i = 0; i < clnt_cnt; i++)
		send(clnt_socks[i], msg, len, 0);
	SetEvent(g_hEvent);		/*设置受信*/
}

void loginVerification(char* name,char* password){
    FILE *fp;
    char username[40],userpassword[40];

    fp=fopen("user.txt","r");
    while(!feof(fp)){
        fscanf(fp,"%s %s",username,userpassword);
        if(strcmp(username, name) == 0 ){
            if(strcmp(userpassword, password) == 0){
                strcpy(loginFeedback,"1");//登录成功

            }else{
                strcpy(loginFeedback,"2");//密码错误
            }
            break;
        }else strcpy(loginFeedback,"0");
    }
}

void readSaveMsg(){
    FILE *fp;
    char info[2050];

    fp=fopen("info.txt","r");
    while(!feof(fp)){
        int i=0;
        fscanf(fp,"%s",info);
        strcat(info, " \n");
        int str_len=strlen(info);
        strcpy(saveMsg[i++],info);
        send_msg(info, str_len);
    }
    fclose(fp);
}

void saveNewMsg(){
    FILE *fp;

    fp=fopen("info.txt","r+");
    for(int i=0;i<msgNumber;i++){
        printf("%s\n",saveMsg[i]);
        fprintf(fp,"%s",saveMsg[i]);
    }
    fclose(fp);
}
void updateMsg(char* msg){
    for(int i=1;i<msgNumber-1;i++){
        strcpy(saveMsg[i-1],saveMsg[i]);
    }
    strcpy(saveMsg[msgNumber-1],msg);
}


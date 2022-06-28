#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
// ������������WS2_32������
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
HANDLE hMutex; //������
char name[NAME_SIZE];
char password[PASSWORD_SIZE];
char msg[BUF_SIZE];
char line1[111]; // �ָ���
char line2[111]; //һ�пհ��ַ�

// ����̨���������ر���
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
	// ��ʼ��WS2_32.dll
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	WSAStartup(sockVersion, &wsaData);

	// �����׽���
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		error_handling("Failed socket()");

	// ��дԶ�̵�ַ��Ϣ
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(8888);
	// �����ļ����û��������ֱ��ʹ�ñ��ص�ַ127.0.0.1
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (connect(sock, (sockaddr*)&servAddr, sizeof(servAddr)) == -1)
		error_handling("Failed connect()");
	//printf("connect success\n");

	//��¼
	int loginFeedback=login();
	if(loginFeedback!=1){
        printf("%d\n",loginFeedback);
        return 0;
	}
    hout = drawConsole();

	hThread[0] = CreateThread(
		NULL,		// Ĭ�ϰ�ȫ����
		NULL,		// Ĭ�϶�ջ��С
		send_msg,	// �߳���ڵ�ַ��ִ���̵߳ĺ�����
		&sock,		// ���������Ĳ���
		0,		// ָ���߳���������
		&dwThreadId);	// �����̵߳�ID��
	hThread[1] = CreateThread(
		NULL,		// Ĭ�ϰ�ȫ����
		NULL,		// Ĭ�϶�ջ��С
		recv_msg,	// �߳���ڵ�ַ��ִ���̵߳ĺ�����
		&sock,		// ���������Ĳ���
		0,		// ָ���߳���������
		&dwThreadId);	// �����̵߳�ID��

	// �ȴ��߳����н���
	WaitForMultipleObjects(2, hThread, true, INFINITE);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	printf(" Thread Over,Enter anything to over.\n");
	getchar();
	// �ر��׽���
	closesocket(sock);
	// �ͷ�WS2_32��
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
		//ȥ������Ŀո�
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
    //���������ҵĵ�¼����
    //system("mode con lines=10 cols=40");
    printf("\n\n                    ��ӭʹ�ö���������\n\n");
    printf("                   �ǳƣ�");
    scanf("%s",name);
    getchar();
    printf("                   ���룺");
    scanf("%s",password);
    getchar();
    //���Լ����ǳƣ����͸�������
    send(sock, name,strlen(name), 0);
    send(sock,password,strlen(password),0);

    recv(sock, msg, sizeof(msg),0);
    if(strcmp(msg ,"0")==0){
        system("cls");
        printf("�û�������\n");
        return 0;
    }else if(strcmp(msg, "2")==0){
        system("cls");
        printf("�������\n");
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

    //���ָ���
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
		// ��Ϣ������й���
		rows = (strLen + width - 1) / width;
		pos.X = 0;
        pos.Y = -rows;
        ScrollConsoleScreenBuffer(hout, &message_region, &message_region, pos, &scroll_fill_char);

		// ���ù��λ��
		pos.X = 0;
        pos.Y = height - INPUT_ROWS - rows - 1;
        FillConsoleOutputCharacter(hout, ' ', width*rows, pos, &num);
		SetConsoleCursorPosition(hout, pos);
	}
}

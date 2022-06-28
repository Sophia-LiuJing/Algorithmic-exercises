#include <winsock2.h>
#include <stdio.h>
#include <windows.h>

// ������������WS2_32������
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
HANDLE hMutex; //������
static POINT pos={0,0};
char name[NAME_SIZE];
char password[PASSWORD_SIZE];
char msg[BUF_SIZE];
char line1[111]; // �ָ���
char line2[111]; //һ�пհ��ַ�

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
	printf("connect success\n");

	//��¼
	int loginFeedback=login();
	if(loginFeedback!=1){
        printf("%d\n",loginFeedback);
        return 0;
	}
    uiInit();

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
    //���������ҵĵ�¼����
    system("mode con lines=10 cols=40");
    printf("\n\n     ��ӭʹ�ö���������\n\n");
    printf("       �ǳƣ�");
    scanf("%s",name);
    getchar();
    printf("       ���룺");
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

void gotoxy(int col,int row){

    HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE);//��ȡ��׼����̨�ľ��
    COORD postem={col,row};
    //����ָ������̨�Ĺ��λ��
    SetConsoleCursorPosition(handle, postem);//�����ĸ�����̨�Ĺ�꣬�ѹ���ƶ����ĸ�λ��

}

void uiInit(){ //��������ʼ��
    system("mode con lines=36 cols=110");
    system("cls"); //�����Ļ
    for(int i=0;i<110;i++){
        line1[i]='-';
    }
    line1[110]=0;
    gotoxy(0,33);//�ƶ����λ��
    printf("%s\n\n",line1);
}

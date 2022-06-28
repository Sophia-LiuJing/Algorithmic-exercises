#include <winsock2.h>	// Ϊ��ʹ��Winsock API����
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#define MAX_CLNT 256
#define BUF_SIZE 100
#define NAME_SIZE 30
#define PASSWORD_SIZE 50
// ������������WS2_32������
#pragma comment(lib,"WS2_32.lib")

void error_handling(const char* msg);		/*��������*/
DWORD WINAPI ThreadProc(LPVOID lpParam);	/*�߳�ִ�к���*/
void send_msg(char* msg, int len);			/*��Ϣ���ͺ���*/
void loginVerification(char* name,char* password);

HANDLE g_hEvent;			/*�¼��ں˶���*/
int clnt_cnt = 0;			//ͳ���׽���
int clnt_socks[MAX_CLNT];	//�����׽���
HANDLE hThread[MAX_CLNT];	//�����߳�
char userName[NAME_SIZE];
char password[PASSWORD_SIZE];
char loginFeedback[2];

int main()
{
	// ��ʼ��WS2_32.dll
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	WSAStartup(sockVersion, &wsaData);	//������һ��2.2�汾��socket

	// ����һ���Զ����õģ�auto-reset events�������ŵģ�signaled���¼��ں˶���
	g_hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

	// �����׽���
	SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serv_sock == INVALID_SOCKET)
		error_handling("Failed socket()");

	// ���sockaddr_in�ṹ
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);				//8888�˿�
	sin.sin_addr.S_un.S_addr = INADDR_ANY;	//���ص�ַ
	//sin.sin_addr.S_un.S_addr = inet_addr("169.254.211.52");

	// ������׽��ֵ�һ�����ص�ַ
	if (bind(serv_sock, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		error_handling("Failed bind()");

	// �������ģʽ
	if (listen(serv_sock, 256) == SOCKET_ERROR)		//���������Ϊ2
		error_handling("Failed listen()");
	printf("Start listen:\n");
	// ѭ�����ܿͻ�����������
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	DWORD dwThreadId;	/*�߳�ID*/
	SOCKET clnt_sock;
	//char szText[] = "hello��\n";
	while (TRUE)
	{
		printf("�ȴ�������\n");
		// ����һ��������
		clnt_sock = accept(serv_sock, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if (clnt_sock == INVALID_SOCKET)
		{
			printf("Failed accept()");
			continue;
		}
		//���������������͵���Ϣ
        recv(clnt_sock, userName, sizeof(userName), 0);
        recv(clnt_sock, password, sizeof(password), 0);
        printf("%s. %s\n",userName,password);
        //�ж��û��Ƿ�����Լ������Ƿ���ȷ
        loginVerification(userName,password);
        send(clnt_sock, loginFeedback, sizeof(loginFeedback), 0);

		/*�ȴ��ں��¼�����״̬����*/
		WaitForSingleObject(g_hEvent, INFINITE);
		hThread[clnt_cnt] = CreateThread(
			NULL,		// Ĭ�ϰ�ȫ����
			NULL,		// Ĭ�϶�ջ��С
			ThreadProc,	// �߳���ڵ�ַ��ִ���̵߳ĺ�����
			(void*)&clnt_sock,		// ���������Ĳ���
			0,		// ָ���߳���������
			&dwThreadId);	// �����̵߳�ID��
		clnt_socks[clnt_cnt++] = clnt_sock;
		SetEvent(g_hEvent);				/*��������*/

		printf(" ���ܵ�һ�����ӣ�%s ִ���߳�ID��%d\r\n", inet_ntoa(remoteAddr.sin_addr), dwThreadId);
	}
	WaitForMultipleObjects(clnt_cnt, hThread, true, INFINITE);
	for (int i = 0; i < clnt_cnt; i++)
	{
		CloseHandle(hThread[i]);
	}
	// �رռ����׽���
	closesocket(serv_sock);
	// �ͷ�WS2_32��
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

	while ((str_len = recv(clnt_sock, msg, sizeof(msg), 0)) != -1)
	{
		send_msg(msg, str_len);
		printf("Ⱥ���ͳɹ�\n");
	}
	printf("�ͻ����˳�:%d\n", GetCurrentThreadId());
	/*�ȴ��ں��¼�����״̬����*/
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
	SetEvent(g_hEvent);		/*��������*/
	// �ر�ͬ�ͻ��˵�����
	closesocket(clnt_sock);
	return NULL;
}

void send_msg(char* msg, int len)
{
	int i;
	/*�ȴ��ں��¼�����״̬����*/
	WaitForSingleObject(g_hEvent, INFINITE);
	for (i = 0; i < clnt_cnt; i++)
		send(clnt_socks[i], msg, len, 0);
	SetEvent(g_hEvent);		/*��������*/
}

void loginVerification(char* name,char* password){
    FILE *fp;
    char username[40],userpassword[40];

    fp=fopen("user.txt","r");
    while(!feof(fp)){
        fscanf(fp,"%s %s",username,userpassword);
        if(strcmp(username, name) == 0 ){
            if(strcmp(userpassword, password) == 0){
                strcpy(loginFeedback,"1");//��¼�ɹ�

            }else{
                strcpy(loginFeedback,"2");//�������
            }
            break;
        }else strcpy(loginFeedback,"0");
    }
}

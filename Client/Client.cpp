#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <string.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable : 4996)

SOCKET SockClient;
void Rev()
{
	char message[1024] = { 0 };
	int re = 0;
	while (1)
	{
		re = 0;
		memset(message, 0, 1024);
		re = recv(SockClient, message, 1024, 0);
		if (re == -1)
			return;
		
		printf("%s\n", message);
	}
}
int main()
{

	//�����׽���
	WSADATA wd;
	WORD wVersion;
	wVersion = MAKEWORD(2, 2);
	if (0 != WSAStartup(wVersion, &wd))
	{
		printf("�����׽��ֿ�ʧ�ܣ�������룺%d\n", GetLastError());
		return 0;
	}

	//�ж�����İ汾�Ƿ�һ��
	if (LOBYTE(wd.wVersion) != 2 || HIBYTE(wd.wVersion) != 2)
	{
		printf("������׽��ְ汾��һ�£�������룺%d\n", GetLastError());
		return 0;
	}

	//�����׽���
	SockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SockClient == INVALID_SOCKET)
	{
		printf("�����׽���ʧ�ܣ�������룺%d", GetLastError());
		return 0;
	}

	SOCKADDR_IN addrSrv;
	//�˴���ip��ַ��Ҫ�Լ��޸�
	char IP[] = "127.0.0.1";
	
	addrSrv.sin_addr.S_un.S_addr = inet_addr(IP);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(9990);

	//���ӷ�����
	if (SOCKET_ERROR == connect(SockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))
	{
		printf("���ӷ�����ʧ�ܣ�������룺%d", GetLastError());
		return 0;
	}
	
	thread rev(Rev);
	rev.detach();
	printf("����		����\n");
	printf("/sign in	��¼�˻�\n");
	printf("/register	ע���˻�	\n");
	printf("/readme		˵����\n");

	char message[1024] = { 0 };
	int n = 1;
	while (n)
	{
		memset(message, 0, 1024);
		fgets(message, 1024, stdin);
		message[strlen(message)-1] = '\0';
		if (strcmp(message, "/readme") == 0)
		{
			printf("����Խ��е����������\n\n");
			printf("/public group	��Ⱥ��\n");
			printf("/join in	����ת��һ����Ⱥ��\n");
			printf("��������join in���Ⱥ��ʱ��\n");
			printf("/yes		ͬ��\n");
			printf("/no		����\n\n");
			printf("/invite		��������ת���Ⱥ��\n");
			printf("���㱻�������Ⱥ��ʱ��\n");
			printf("/accept		ͬ������\n");
			printf("/refuse		�ܾ�\n\n");
			printf("/quit		�˳���ǰȺ��\n");
			printf("/sign in	��¼�˻�\n");
			printf("/register	ע���˻�	\n");
			printf("/clean		���ڰ�\n");
		}
		else if (strcmp(message, "/clean") == 0)
		{
			system("cls");
		}
		else
		{
			send(SockClient, message, strlen(message), 0);
		}
		
	}

	return 0;
}
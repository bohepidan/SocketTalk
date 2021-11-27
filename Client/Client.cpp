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

	//加载套接字
	WSADATA wd;
	WORD wVersion;
	wVersion = MAKEWORD(2, 2);
	if (0 != WSAStartup(wVersion, &wd))
	{
		printf("加载套接字库失败，错误代码：%d\n", GetLastError());
		return 0;
	}

	//判断请求的版本是否一致
	if (LOBYTE(wd.wVersion) != 2 || HIBYTE(wd.wVersion) != 2)
	{
		printf("请求的套接字版本不一致，错误代码：%d\n", GetLastError());
		return 0;
	}

	//创建套接字
	SockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SockClient == INVALID_SOCKET)
	{
		printf("创建套接字失败，错误代码：%d", GetLastError());
		return 0;
	}

	SOCKADDR_IN addrSrv;
	//此处的ip地址需要自己修改
	char IP[] = "127.0.0.1";
	
	addrSrv.sin_addr.S_un.S_addr = inet_addr(IP);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(9990);

	//链接服务器
	if (SOCKET_ERROR == connect(SockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)))
	{
		printf("连接服务器失败，错误代码：%d", GetLastError());
		return 0;
	}
	
	thread rev(Rev);
	rev.detach();
	printf("输入		功能\n");
	printf("/sign in	登录账户\n");
	printf("/register	注册账户	\n");
	printf("/readme		说明书\n");

	char message[1024] = { 0 };
	int n = 1;
	while (n)
	{
		memset(message, 0, 1024);
		fgets(message, 1024, stdin);
		message[strlen(message)-1] = '\0';
		if (strcmp(message, "/readme") == 0)
		{
			printf("你可以进行的命令操作：\n\n");
			printf("/public group	大群聊\n");
			printf("/join in	请求转入一个新群聊\n");
			printf("有人请求join in你的群聊时：\n");
			printf("/yes		同意\n");
			printf("/no		滚蛋\n\n");
			printf("/invite		邀请他人转入该群聊\n");
			printf("当你被邀请加入群聊时：\n");
			printf("/accept		同上贼船\n");
			printf("/refuse		拒绝\n\n");
			printf("/quit		退出当前群聊\n");
			printf("/sign in	登录账户\n");
			printf("/register	注册账户	\n");
			printf("/clean		擦黑板\n");
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
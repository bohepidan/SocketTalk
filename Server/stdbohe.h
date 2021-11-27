#pragma once
// TcpServer.cpp : 定义控制台应用程序的入口点。
//

#include <stdlib.h>
#include <WINSOCK2.H>   
#include <stdio.h>
#include <string.h>
#include <WS2tcpip.h>
#pragma comment(lib,"WS2_32.lib")   
#include <time.h>
#define   PORT   9990   
#define   DATA_BUFSIZE   8192  
#define   NAME_SIZE 100
#define   PASSWORD_SIZE 100

typedef struct _grouP* GroupList;  //用一个链表来表示一个人所在的群聊
// 定义套接字信息
typedef   struct   _SOCKET_INFORMATION {
    CHAR   Buffer[DATA_BUFSIZE];        // 发送和接收数据的缓冲区
    WSABUF   DataBuf;                       // 定义发送和接收数据缓冲区的结构体，包括缓冲区的长度和内容
    SOCKET   Socket;                            // 与客户端进行通信的套接字
    DWORD   BytesSEND;                  // 保存套接字发送的字节数
    DWORD   BytesRECV;                  // 保存套接字接收的字节数
    GroupList grouphead;               //群聊链表头，为哨兵
    long inviter;                          //邀请人
    long joiner;                            //请求加入者
    long account;                       //用户账号
    char name[NAME_SIZE];           //用户名
    int command;                    //客户端发送的指令代号
    int situation;                      //  在指令内细分操作的代号
}SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

struct _grouP {
    LPSOCKET_INFORMATION pmember;
    GroupList next;
}Group;

int Register(char name[], char password[], long account);
void Connect(LPSOCKET_INFORMATION SocketInfo_1, LPSOCKET_INFORMATION SocketInfo_2);
void JoinIn(LPSOCKET_INFORMATION SocketInfo_1, LPSOCKET_INFORMATION SocketInfo_2);
int SwitchCommand(LPSOCKET_INFORMATION SocketInfo);
char* FindAccount(LPSOCKET_INFORMATION SocketInfo, long goal);
BOOL   CreateSocketInformation(SOCKET   s);
void   FreeSocketInformation(DWORD   Index);
void QuitGroup(LPSOCKET_INFORMATION SI);

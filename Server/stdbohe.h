#pragma once
// TcpServer.cpp : �������̨Ӧ�ó������ڵ㡣
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

typedef struct _grouP* GroupList;  //��һ����������ʾһ�������ڵ�Ⱥ��
// �����׽�����Ϣ
typedef   struct   _SOCKET_INFORMATION {
    CHAR   Buffer[DATA_BUFSIZE];        // ���ͺͽ������ݵĻ�����
    WSABUF   DataBuf;                       // ���巢�ͺͽ������ݻ������Ľṹ�壬�����������ĳ��Ⱥ�����
    SOCKET   Socket;                            // ��ͻ��˽���ͨ�ŵ��׽���
    DWORD   BytesSEND;                  // �����׽��ַ��͵��ֽ���
    DWORD   BytesRECV;                  // �����׽��ֽ��յ��ֽ���
    GroupList grouphead;               //Ⱥ������ͷ��Ϊ�ڱ�
    long inviter;                          //������
    long joiner;                            //���������
    long account;                       //�û��˺�
    char name[NAME_SIZE];           //�û���
    int command;                    //�ͻ��˷��͵�ָ�����
    int situation;                      //  ��ָ����ϸ�ֲ����Ĵ���
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

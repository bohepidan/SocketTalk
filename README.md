#SocketTal<br>
  一个刚学会c语言不久的菜鸟的练手项目，瑕疵颇多，但至少能正常完成一个聊天程序的基本操作。<br>
以下为一个用户套接字的结构体<br>
  ```c
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
```
其中使用一个包含用户结构体指针的链表来表示用户所在的群聊，通过这种方式实现了诸如<br>
加入群聊的函数<br>
```c
//当用户要加入的群聊中只有一个人时
void Connect(LPSOCKET_INFORMATION SocketInfo_1, LPSOCKET_INFORMATION SocketInfo_2) {
	GroupList member_1 = malloc(sizeof(struct _grouP));
	GroupList member_2 = malloc(sizeof(struct _grouP));
	member_1->pmember = SocketInfo_1;
	member_2->pmember = SocketInfo_2;
	member_1->next = member_2;
	member_2->next = NULL;
	SocketInfo_2->grouphead = SocketInfo_1->grouphead;
	SocketInfo_1->grouphead->next = member_1;
}

//当用户要加入的群聊中有多个人时
void JoinIn(LPSOCKET_INFORMATION SocketInfo_1, LPSOCKET_INFORMATION SocketInfo_2) {
	GroupList newmember = malloc(sizeof(struct _grouP));
	newmember->next = NULL;
	newmember->pmember = SocketInfo_2;
	GroupList here = SocketInfo_1->grouphead->next;
	while (here->next) {
		here = here->next;
	}
	here->next = newmember;
	SocketInfo_2->grouphead->next = SocketInfo_1->grouphead->next;
}
```
退出群聊的函数<br>
```c
void QuitGroup(LPSOCKET_INFORMATION SI) {
	if (SI->grouphead->next != NULL) {
		//当用户正在一个群内的时候，需要将用户从群内退出
		char tmpstr[DATA_BUFSIZE];
		char tmpstr2[DATA_BUFSIZE];
		GroupList tmphere = NULL;
		GroupList tmphere2 = NULL;
		strcpy_s(tmpstr, DATA_BUFSIZE, SI->name);
		strcat_s(tmpstr, DATA_BUFSIZE, "(");
		_itoa_s(SI->account, tmpstr2, DATA_BUFSIZE, 10);
		strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
		strcat_s(tmpstr, DATA_BUFSIZE, ")---");
		strcat_s(tmpstr, DATA_BUFSIZE, "退出群聊！\n");
		strcpy_s(SI->DataBuf.buf, DATA_BUFSIZE, tmpstr);
		tmphere = SI->grouphead->next;
		while (tmphere) {
			//为群聊内的每一位成员发送消息
			strcpy_s(tmphere->pmember->DataBuf.buf, DATA_BUFSIZE, tmpstr);
			tmphere->pmember->BytesRECV = strlen(tmphere->pmember->DataBuf.buf);
			tmphere = tmphere->next;
		}
		GroupList tmpmember = SI->grouphead;
		if (SI->grouphead->next->next->next == NULL) {
			//群内只有两个人
			tmphere = SI->grouphead->next->next;
			tmphere2 = SI->grouphead->next;
			tmphere->next = NULL;
			tmphere2->next = NULL;
			//这里需要思考
			//talk.c 是最后的修改
			tmphere->pmember->grouphead = tmphere;
			tmphere2->pmember->grouphead = tmphere2;
		}
		else {
			//群内有多个人
			while (tmpmember) {
				if (tmpmember->next->pmember == SI) {
					//将用户退出
					tmphere = tmpmember->next;
					tmpmember->next = tmpmember->next->next;
					break;
				}
				tmpmember = tmpmember->next;
			}
			//将用户置于空群聊
			SI->grouphead = tmphere;
			tmphere->next = NULL;
		}
	}
}
```
用户注册的信息会保存在服务器的Members.txt中，这样可以实现用户的下一次登录。<br>
通过以上关键代码以及其他的一些代码，实现了双人或多人远程聊天，并且可以同时存在多个群聊，群聊中的人数不定。

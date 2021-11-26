#include "stdbohe.h"
DWORD   TotalSockets = 0;               // 记录正在使用的套接字总数量
LPSOCKET_INFORMATION    SocketArray[FD_SETSIZE];         // 保存Socket信息对象的数组，FD_SETSIZE表示SELECT
//模型中允许的最大套接字数量

// 创建SOCKET信息
BOOL   CreateSocketInformation(SOCKET   s)
{
    LPSOCKET_INFORMATION   SI;                                      // 用于保存套接字的信息       
 //   printf("Accepted   socket   number   %d\n",   s);         // 打开已接受的套接字编号
    // 为SI分配内存空间
    if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        printf("GlobalAlloc()   failed   with   error   %d\n", GetLastError());
        return   FALSE;
    }
    // 初始化SI的值    
    SI->Socket = s;
    SI->grouphead = malloc(sizeof(Group));
    SI->BytesSEND = 0;
    SI->BytesRECV = 0;
    SI->inviter = 0;
    SI->grouphead->pmember = NULL;
    SI->account = 0;
    SI->grouphead->next = NULL;
    SI->command = 0;
    SI->situation = 0;
    // 在SocketArray数组中增加一个新元素，用于保存SI对象 
    SocketArray[TotalSockets] = SI;
    TotalSockets++;                     // 增加套接字数量

    return(TRUE);
}

// 从数组SocketArray中删除指定的LPSOCKET_INFORMATION对象
void   FreeSocketInformation(DWORD   Index)
{
    LPSOCKET_INFORMATION SI = SocketArray[Index];   // 获取指定索引对应的LPSOCKET_INFORMATION对象
    DWORD   i;
    // 关闭套接字
    QuitGroup(SI);
    closesocket(SI->Socket);
    //printf("Closing   socket   number   %d\n",   SI->Socket);   
    // 释放指定LPSOCKET_INFORMATION对象资源
    GlobalFree(SI);
    // 将数组中index索引后面的元素前移
    for (i = Index; i < TotalSockets; i++)
    {
        SocketArray[i] = SocketArray[i + 1];
    }
    TotalSockets--;        // 套接字总数减1
}

// 主函数，启动服务器
int main()
{
    SOCKET   ListenSocket;                  // 监听套接字
    SOCKET   AcceptSocket;                  // 与客户端进行通信的套接字
    SOCKADDR_IN   InternetAddr;         // 服务器的地址
    WSADATA   wsaData;                      // 用于初始化套接字环境
    INT   Ret;                                          // WinSock API的返回值
    FD_SET   WriteSet;                          // 获取可写性的套接字集合
    FD_SET   ReadSet;                           // 获取可读性的套接字集合
    DWORD   Total = 0;                              // 处于就绪状态的套接字数量
    DWORD   SendBytes;                      // 发送的字节数
    DWORD   RecvBytes;                      // 接收的字节数
    char NAME[NAME_SIZE] = "";                 //用户名
    char PASSWORD[PASSWORD_SIZE] = "";           //密码 
    long ACCOUNT = 0;                          //账号
    char tmpstr[DATA_BUFSIZE] = "";              //一个临时字符串
    char tmpstr2[DATA_BUFSIZE] = "";
    int isfound = 0; 
    GroupList tmphere = NULL;              //一个临时群聊成员指针
    LPSOCKET_INFORMATION tmpSocketInfo = NULL;
    LPSOCKET_INFORMATION tmpSocketInfo2 = NULL ;
    int sendcase = 1;                         //分辨消息是否 1-只发送给自己 2-发送给包括自己的群聊中的所有人 3-发送给除了自己
                                                //以外的群聊中的所有人

    // 初始化WinSock环境
    if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
    {
        printf("WSAStartup()   failed   with   error   %d\n", Ret);
        WSACleanup();
        return -1;
    }
    // 创建用于监听的套接字 
    if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("WSASocket()   failed   with   error   %d\n", WSAGetLastError());
        return -1;
    }
    // 设置监听地址和端口号
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT);
    // 绑定监听套接字到本地地址和端口
    if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        printf("bind()   failed   with   error   %d\n", WSAGetLastError());
        return -1;
    }
    // 开始监听
    if (listen(ListenSocket, 5))
    {
        printf("listen()   failed   with   error   %d\n", WSAGetLastError());
        return -1;
    }


     //设置为非阻塞模式
    ULONG NonBlock = 1;
    if(ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
    {
        printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
        return -1;
    }
        // 为ListenSocket套接字创建对应的SOCKET_INFORMATION
        // 这样就可以把ListenSocket添加到SocketArray数组中
    CreateSocketInformation(ListenSocket);
    while (TRUE)
    {
        // 准备用于网络I/O通知的读/写套接字集合
        FD_ZERO(&ReadSet);
        FD_ZERO(&WriteSet);
        // 向ReadSet集合中添加监听套接字ListenSocket
        FD_SET(ListenSocket, &ReadSet);
        // 将SocketArray数组中的所有套接字添加到WriteSet和ReadSet集合中
        // SocketArray数组中保存着监听套接字和所有与客户端进行通信的套接字
        // 这样就可以使用select()判断哪个套接字有接入数据或者读取/写入数据
        for (DWORD i = 0; i < TotalSockets; i++)
        {
            LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
            FD_SET(SocketInfo->Socket, &WriteSet);
            FD_SET(SocketInfo->Socket, &ReadSet);
        }
        // 判断读/写套接字集合中就绪的套接字    
        if ((Total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR)
        {
            printf("select()   returned   with   error   %d\n", WSAGetLastError());
            return -1;
        }
        // 依次处理所有套接字。本服务器是一个回应服务器，即将从客户端收到的字符串再发回到客户端。
        for (DWORD i = 0; i < TotalSockets; i++)
        {
            LPSOCKET_INFORMATION SocketInfo = SocketArray[i];           // SocketInfo为当前要处理的套接字信息
            // 判断当前套接字的可读性，即是否有接入的连接请求或者可以接收数据
            if (FD_ISSET(SocketInfo->Socket, &ReadSet))
            {
                if (SocketInfo->Socket == ListenSocket)      // 对于监听套接字来说，可读表示有新的连接请求
                {
                    Total--;    // 就绪的套接字减1
                    // 接受连接请求，得到与客户端进行通信的套接字AcceptSocket
                    if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
                    {
                        printf("socket:%u 连接成功！\n",AcceptSocket);
                        // 设置套接字AcceptSocket为非阻塞模式
                        // 这样服务器在调用WSASend()函数发送数据时就不会被阻塞
                        NonBlock   =   1;
                        if(ioctlsocket(AcceptSocket, FIONBIO, &NonBlock)   ==   SOCKET_ERROR)
                        {
                            printf("ioctlsocket()   failed   with   error   %d\n",   WSAGetLastError());
                            return -1;
                        }
                        // 创建套接字信息，初始化LPSOCKET_INFORMATION结构体数据，将AcceptSocket添加到SocketArray数组中
                        if (CreateSocketInformation(AcceptSocket) == FALSE)
                            return -1;
                    }
                    else
                    {
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            printf("accept()   failed   with   error   %d\n", WSAGetLastError());
                            return -1;
                        }
                    }
                }
                else   // 接收数据
                {
                    // 如果当前套接字在ReadSet集合中，则表明该套接字上有可以读取的数据
                    if (FD_ISSET(SocketInfo->Socket, &ReadSet))
                    {
                        Total--;                // 减少一个处于就绪状态的套接字
                        memset(SocketInfo->Buffer, ' ', DATA_BUFSIZE);
                        SocketInfo->DataBuf.buf = SocketInfo->Buffer;           // 初始化缓冲区位置
                        SocketInfo->DataBuf.len = DATA_BUFSIZE;             // 初始化缓冲区长度
                        ZeroMemory(SocketInfo->DataBuf.buf, DATA_BUFSIZE);// 初始化缓冲区
                        // 接收数据
                        DWORD  Flags = 0;
                        if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
                            NULL, NULL) == SOCKET_ERROR)
                        {
                            // 错误编码等于WSAEWOULDBLOCK表示暂没有数据，否则表示出现异常
                            if (WSAGetLastError() != WSAEWOULDBLOCK)
                            {
                                printf("WSARecv()   failed   with   error   %d\n", WSAGetLastError());
                                FreeSocketInformation(i);       // 释放套接字信息
                            }
                            continue;
                        }
                        else   // 接收数据
                        {
                            switch (SwitchCommand(SocketInfo)) {
                                //用户注册 /register
                                case 1:
                                    switch (SocketInfo->situation) {
                                        case 0:
                                            SocketInfo->command = 1;
                                            //当situation为零时，说明用户尚未开始注册
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "请输入你的用户名(name)：");
                                            (SocketInfo->situation)++;      //situation加一
                                            sendcase = 1;
                                            break;
                                        case 1:
                                            //situation为1，说明用户输入了用户名，存在Databuf.buf内
                                            strcpy_s(NAME, NAME_SIZE, SocketInfo->DataBuf.buf);
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "请输入你的密码(password)：");
                                            (SocketInfo->situation)++;
                                            sendcase = 1;
                                            break;
                                        case 2:
                                            //此时用户已输入了密码
                                            strcpy_s(PASSWORD, PASSWORD_SIZE, SocketInfo->DataBuf.buf);
                                            //随机一个七位数账号  
                                            //***得到的ACCOUNT可能是负值，因此调用时需要用%ul而非%l***
                                            srand(time(NULL));
                                            ACCOUNT = (rand() * 531 + 100000) % 1000000;
                                            //在服务器的Members.txt文件中录入这个账户
                                            if (Register(NAME, PASSWORD, ACCOUNT) == 0) {
                                                _itoa_s(ACCOUNT, tmpstr, DATA_BUFSIZE, 10);
                                                strcat_s(tmpstr, DATA_BUFSIZE, "为你的账号! 请输入/sign in以登陆! OvO");
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "注册成功！\n");
                                                strcat_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                sendcase = 1;
                                            }
                                            else {
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "注册失败！");
                                                sendcase = 1;
                                            }
                                            SocketInfo->situation = 0;
                                            SocketInfo->command = 0;
                                    }
                                    break;
                                    //用户登陆 /sign in
                                case 2:
                                    switch (SocketInfo->situation) {
                                        case 0:
                                            SocketInfo->command = 2;
                                            //当situation为零时，说明用户尚未开始登陆
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "请输入你的账号(account)：");
                                            (SocketInfo->situation)++;      //situation加一
                                            sendcase = 1;
                                            break;
                                        case 1:
                                            //此时用户输入了账号，存在Databuf.buf内
                                            ACCOUNT = atoi(SocketInfo->DataBuf.buf);
                                            //在members.txt文件内寻找该账号
                                            isfound = 0;
                                            if (ACCOUNT == 0) {
                                                ACCOUNT = -1;
                                            }
                                            for (DWORD i = 0; i < TotalSockets; i++)
                                            {
                                                tmpSocketInfo = SocketArray[i];
                                                if (tmpSocketInfo->account == ACCOUNT) {
                                                    isfound = 1;
                                                    break;
                                                }
                                            }
                                            if (isfound) {
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "该用户已在线，请勿重复登录！");
                                                sendcase = 1;
                                                SocketInfo->situation = 0;
                                                SocketInfo->command = 0;
                                                break;
                                            }
                                            strcpy_s(PASSWORD, PASSWORD_SIZE, FindAccount(SocketInfo, ACCOUNT));
                                            if (strcmp(PASSWORD, "notfound") == 0) {
                                                //findaccount函数返回一个字符串，如果未找到账号，则返回"notfound"
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "未找到该账号！");
                                                //重置socket结构体的状态并且break
                                                SocketInfo->situation = 0;
                                                SocketInfo->command = 0;
                                                sendcase = 1;
                                            }
                                            else {
                                                //找到该用户，则继续要求用户输入密码
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "请输入密码(password):");
                                                (SocketInfo->situation)++;
                                                sendcase = 1;
                                            }
                                            break;
                                        case 2:
                                            //此时用户输入了密码,存在Databuf.buf内
                                            if (strcmp(PASSWORD, SocketInfo->DataBuf.buf) == 0) {
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "密码正确！\n");
                                                strcat_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, SocketInfo->name);
                                                strcat_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, ",你已成功登陆！\n你现在可以使用/invite、/join in和/public group以开始聊天(如果不知道指令的作用，请使用/readme)^^");

                                                //用户登陆成功，开始talk操作  
                                                SocketInfo->situation = 0;
                                                SocketInfo->command = 5;  
                                                sendcase = 1;
                                            }
                                            else {
                                                //密码不符，则重置socket结构体的状态并且break
                                                SocketInfo->account = 0;
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "密码错误！");
                                                SocketInfo->situation = 0;
                                                SocketInfo->command = 0;
                                                sendcase = 1;
                                            }
                                    }
                                    break;
                                    //寻找用户并建立连接，开始聊天 /talk
                                case 3:
                                    switch (SocketInfo->situation) {
                                        case 0:
                                            SocketInfo->command = 3;
                                            //此时用户已经登陆 command == 3 ,并且输入了 /talk
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "请输入好友的账号(account):");
                                            (SocketInfo->situation)++;
                                            sendcase = 1;
                                            break;
                                        case 1:
                                            //此时用户输入了好友的账号(account)，并且保存在databuf.buf内
                                            ACCOUNT = atoi(SocketInfo->DataBuf.buf);
                                            if (ACCOUNT == 0) {
                                                ACCOUNT = -1;
                                            }
                                            printf("开始寻找好友%d\n",ACCOUNT);
                                            isfound = 0;
                                            for (DWORD i = 0; i < TotalSockets; i++) {
                                                tmpSocketInfo = SocketArray[i];
                                                printf("找到一个人%d\n", tmpSocketInfo->account);
                                                if (tmpSocketInfo->account == ACCOUNT) {
                                                    //此时在在线用户中找到了目标用户
                                                    printf("找到目标！\n");
                                                    isfound = 1;
                                                    if (tmpSocketInfo->grouphead->next == NULL) {
                                                        strcpy_s(tmpstr, DATA_BUFSIZE, SocketInfo->name);
                                                        strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                                        _itoa_s(SocketInfo->account, tmpstr2, DATA_BUFSIZE, 10);
                                                        strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                                        strcat_s(tmpstr, DATA_BUFSIZE, ")邀请你加入他的群聊，同意请输入/accept, 否则输入/refuse");
                                                        strcpy_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                        tmpSocketInfo->BytesRECV = strlen(tmpSocketInfo->DataBuf.buf);
                                                        tmpSocketInfo->inviter = SocketInfo->account;
                                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "已发送邀请！\n");
                                                        SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                                    }
                                                    else {
                                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "对方已经在一个群内！\n");
                                                        SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                                        SocketInfo->command = 5;
                                                    }
                                                }
                                                
                                            }
                                            if (isfound == 0) {
                                                //未找到目标用户
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "未找到好友，好友可能不在线或者输入的账号(account)不存在...TAT");
                                                sendcase = 1;
                                                SocketInfo->situation = 0;
                                            }
                                            // 最后回到聊天间
                                            SocketInfo->command = 5;
                                            SocketInfo->situation = 0;
                                    }
                                    break;
                                    //无效指令
                                case 4:
                                    strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "无效的指令！");
                                    sendcase = 1;
                                    break;
                                case 5:
                                    if (SocketInfo->grouphead->next == NULL) {
                                        //用户未进入聊天
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "你不在任何一个群内...建议入群后再聊天~~");
                                        sendcase = 1;
                                    }
                                    else {
                                        //用户开始聊天，并且输入的不是'/'开头的指令符
                                        sendcase = 3;
                                    }
                                    break;
                                case 6:
                                    strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "请先登陆！");
                                    sendcase = 1;
                                    break;
                                case 7:
                                    //接受邀请
                                    if (SocketInfo->inviter == 0) {
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "没有人邀请你... 0.0");
                                        SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                        sendcase = 1;
                                    }
                                    else {
                                        isfound = 0;
                                        ACCOUNT = SocketInfo->inviter;
                                        printf("开始寻找邀请者...\n");
                                        for (DWORD i = 0; i < TotalSockets; i++) {
                                            tmpSocketInfo = SocketArray[i];
                                            if (tmpSocketInfo->account == ACCOUNT) {
                                                //此时在在线用户中找到了邀请者
                                                printf("找到目标！\n");
                                                isfound = 1;
                                                if (SocketInfo->grouphead->next != NULL) {
                                                    //如果在一个群内，/accept需要先退出该群
                                                    QuitGroup(SocketInfo);
                                                    isfound = 2;
                                                }
                                                if (NULL == tmpSocketInfo->grouphead->next) {
                                                    //邀请者尚未和任何人建立连接
                                                    Connect(tmpSocketInfo, SocketInfo);
                                                }
                                                else {
                                                    //邀请者已建立一个群聊
                                                    JoinIn(tmpSocketInfo, SocketInfo);
                                                }
                                                strcpy_s(tmpstr, DATA_BUFSIZE, SocketInfo->name);
                                                strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                                _itoa_s(SocketInfo->account, tmpstr2, DATA_BUFSIZE, 10);
                                                strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                                strcat_s(tmpstr, DATA_BUFSIZE, ")---");
                                                strcat_s(tmpstr, DATA_BUFSIZE, "加入群聊！\n");
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                tmphere = SocketInfo->grouphead->next;
                                                while (tmphere) {
                                                    //为群聊内的每一位成员发送消息
                                                    strcpy_s(tmphere->pmember->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                    tmphere->pmember->BytesRECV = strlen(tmphere->pmember->DataBuf.buf);
                                                    //让这些成员把这些信息仅发给自己
                                                    tmphere = tmphere->next;
                                                }
                                                if (isfound == 2) {
                                                    strcpy_s(tmpstr, DATA_BUFSIZE, "你已退出当前的群聊!\n");
                                                    strcat_s(tmpstr, DATA_BUFSIZE, SocketInfo->DataBuf.buf);
                                                    strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                }
                                                sendcase = 1;
                                                //进入聊天间
                                                SocketInfo->command = 5;
                                            }
                                        }
                                        if (isfound == 0) {
                                            //未找到目标用户
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "未找到好友，好友可能不在线或者输入的账号(account)不存在...TAT");
                                            sendcase = 1;
                                            SocketInfo->situation = 0;
                                        }
                                        //重置inviter
                                        SocketInfo->inviter = 0;
                                    }
                                    break;
                                case 8:
                                    //拒绝邀请
                                    if (SocketInfo->inviter == 0) {
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "本就没有人邀请你... 0.0");
                                        SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                        sendcase = 1;
                                    }
                                    else {
                                        isfound = 0;
                                        ACCOUNT = SocketInfo->inviter;
                                        printf("开始寻找邀请者...\n");
                                        for (DWORD i = 0; i < TotalSockets; i++) {
                                            tmpSocketInfo = SocketArray[i];
                                            if (tmpSocketInfo->account == ACCOUNT) {
                                                //此时在在线用户中找到了邀请者
                                                printf("找到目标！\n");
                                                isfound = 1;
                                                strcpy_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, SocketInfo->name);
                                                strcat_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, "(");
                                                _itoa_s(SocketInfo->account, tmpstr, DATA_BUFSIZE, 10);
                                                strcat_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                strcat_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, ")拒绝了你的邀请！");

                                                tmpSocketInfo->situation = -1;

                                                tmpSocketInfo->BytesRECV = strlen(tmpSocketInfo->DataBuf.buf);
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "已拒绝！");
                                                SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                                sendcase = 1;
                                            }
                                        }
                                        if (isfound == 0) {
                                            //未找到目标用户
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "拒绝的消息未成功发送给好友，好友可能不在线...TAT");
                                            sendcase = 1;
                                            SocketInfo->situation = 0;
                                        }
                                    }
                                    SocketInfo->inviter = 0;
                                    break;
                                case 9:
                                    //消息仅发送给自己
                                    SocketInfo->situation = 0;
                                    sendcase = 1;
                                    break;
                                case 10:
                                    //退出群
                                    if (SocketInfo->grouphead->next == NULL) {
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "你不在任何一个群内！");
                                        sendcase = 1;
                                    }
                                    else {
                                        QuitGroup(SocketInfo);
                                    }
                                    SocketInfo->command = 5;
                                    break;
                                case 11:
                                    switch (SocketInfo->situation) {
                                        case 0:
                                            SocketInfo->command = 11;
                                            //此时用户已经登陆 command == 3 ,并且输入了 /join in
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "请输入好友的账号(account):");
                                            (SocketInfo->situation)++;
                                            sendcase = 1;
                                            break;
                                        case 1:
                                            //此时用户输入了好友的账号(account)，并且保存在databuf.buf内
                                            ACCOUNT = atoi(SocketInfo->DataBuf.buf);
                                            if (ACCOUNT == 0) {
                                                ACCOUNT = -1;
                                            }
                                            printf("开始寻找好友%d\n", ACCOUNT);
                                            isfound = 0;
                                            for (DWORD i = 0; i < TotalSockets; i++) {
                                                tmpSocketInfo = SocketArray[i];
                                                printf("找到一个人%d\n", tmpSocketInfo->account);
                                                if (tmpSocketInfo->account == ACCOUNT) {
                                                    //此时在在线用户中找到了目标用户
                                                    printf("找到目标！\n");
                                                    isfound = 1;
                                                    strcpy_s(tmpstr, DATA_BUFSIZE, SocketInfo->name);
                                                    strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                                    _itoa_s(SocketInfo->account, tmpstr2, DATA_BUFSIZE, 10);
                                                    strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                                    strcat_s(tmpstr, DATA_BUFSIZE, ")请求加入你的群聊，同意请输入/yes, 否则输入/no");
                                                    strcpy_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                    tmpSocketInfo->BytesRECV = strlen(tmpSocketInfo->DataBuf.buf);
                                                    tmpSocketInfo->joiner = SocketInfo->account;
                                                    strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "已发送请求！\n");
                                                    SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                                }

                                            }
                                            if (isfound == 0) {
                                                //未找到目标用户
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "未找到好友，好友可能不在线或者输入的账号(account)不存在...TAT\n");
                                                sendcase = 1;
                                                SocketInfo->situation = 0;
                                            }
                                            // 最后回到聊天间
                                            SocketInfo->command = 5;
                                            SocketInfo->situation = 0;
                                    }
                                    break;
                                case 12:
                                    //接受好友的入群请求
                                    if (SocketInfo->joiner == 0) {
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "没有人请求入群... 0.0");
                                        SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                        sendcase = 1;
                                    }
                                    else {
                                        isfound = 0;
                                        ACCOUNT = SocketInfo->joiner;
                                        printf("开始寻找请求入群者...\n");
                                        for (DWORD i = 0; i < TotalSockets; i++) {
                                            tmpSocketInfo = SocketArray[i];
                                            if (tmpSocketInfo->account == ACCOUNT) {
                                                //此时在在线用户中找到了请求者
                                                printf("找到目标！\n");
                                                isfound = 1;
                                                if (tmpSocketInfo->grouphead->next != NULL) {
                                                    //如果在一个群内，请求者需要先退出该群
                                                    QuitGroup(tmpSocketInfo);
                                                    isfound = 2;
                                                }
                                                if (NULL == SocketInfo->grouphead->next) {
                                                    //对方尚未和任何人建立连接
                                                    Connect(SocketInfo, tmpSocketInfo);
                                                }
                                                else {
                                                    //对方已建立一个群聊
                                                    JoinIn(SocketInfo, tmpSocketInfo);
                                                }
                                                strcpy_s(tmpstr, DATA_BUFSIZE, SocketInfo->name);
                                                strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                                _itoa_s(SocketInfo->account, tmpstr2, DATA_BUFSIZE, 10);
                                                strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                                strcat_s(tmpstr, DATA_BUFSIZE, ")---");
                                                strcat_s(tmpstr, DATA_BUFSIZE, "加入群聊！\n");
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                tmphere = SocketInfo->grouphead->next;
                                                while (tmphere) {
                                                    //为群聊内的每一位成员发送消息
                                                    strcpy_s(tmphere->pmember->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                    tmphere->pmember->BytesRECV = strlen(tmphere->pmember->DataBuf.buf);
                                                    //让这些成员把这些信息仅发给自己
                                                    tmphere = tmphere->next;
                                                }
                                                strcpy_s(tmpstr, DATA_BUFSIZE, tmpSocketInfo->name);
                                                strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                                _itoa_s(tmpSocketInfo->account, tmpstr2, DATA_BUFSIZE, 10);
                                                strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                                strcat_s(tmpstr, DATA_BUFSIZE, ")接受了你的入群邀请！\n");
                                                if (isfound == 2) {
                                                    strcat_s(tmpstr, DATA_BUFSIZE, "你已退出当前的群聊!\n");
                                                }
                                                strcat_s(tmpstr, DATA_BUFSIZE, SocketInfo->DataBuf.buf);
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                sendcase = 1;
                                                //进入聊天间
                                                SocketInfo->command = 5;
                                            }
                                        }
                                        if (isfound == 0) {
                                            //未找到目标用户
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "未找到好友，好友可能不在线或者输入的账号(account)不存在...TAT");
                                            sendcase = 1;
                                            SocketInfo->situation = 0;
                                        }
                                        //重置joiner
                                        SocketInfo->joiner = 0;
                                    }
                                    break;
                                case 13:
                                    if (SocketInfo->joiner == 0) {
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "没有人请求入群... 0.0");
                                        SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                        sendcase = 1;
                                    }
                                    else {
                                        isfound = 0;
                                        ACCOUNT = SocketInfo->joiner;
                                        printf("开始寻找请求入群者...\n");
                                        for (DWORD i = 0; i < TotalSockets; i++) {
                                            tmpSocketInfo = SocketArray[i];
                                            if (tmpSocketInfo->account == ACCOUNT) {
                                                //此时在在线用户中找到了邀请者
                                                printf("找到目标！\n");
                                                isfound = 1;
                                                strcpy_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, SocketInfo->name);
                                                strcat_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, "(");
                                                _itoa_s(SocketInfo->account, tmpstr, DATA_BUFSIZE, 10);
                                                strcat_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                                strcat_s(tmpSocketInfo->DataBuf.buf, DATA_BUFSIZE, ")拒绝了你的入群请求！");
                                                tmpSocketInfo->BytesRECV = strlen(tmpSocketInfo->DataBuf.buf);
                                                strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "已拒绝！");
                                                SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);
                                                sendcase = 1;
                                            }
                                        }
                                        if (isfound == 0) {
                                            //未找到目标用户
                                            strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "拒绝的消息未成功发送给好友，好友可能不在线...TAT");
                                            sendcase = 1;
                                            SocketInfo->situation = 0;
                                        }
                                    }
                                    SocketInfo->joiner = 0;
                                    break;
                                case 14:
                                    isfound = 0;
                                    for (DWORD i = 0; i < TotalSockets; i++)
                                    {
                                        tmpSocketInfo = SocketArray[i];
                                        if (tmpSocketInfo->account == 111) {
                                            isfound = 1;
                                            break;
                                        }
                                    }
                                    if (SocketInfo->grouphead->next == tmpSocketInfo->grouphead->next && tmpSocketInfo->grouphead->next != NULL) {
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "你已在大群！");
                                        sendcase = 1;
                                        SocketInfo->command = 5;
                                        break;
                                    }
                                    if (isfound == 0) {
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, "大群未开放！");
                                        sendcase = 1;
                                        SocketInfo->command = 5;
                                        break;
                                    }
                                    if (SocketInfo->grouphead->next != NULL) {
                                        //已在一个群则退出当前群聊
                                        QuitGroup(SocketInfo);
                                        isfound = 2;
                                    }

                                    if (NULL == tmpSocketInfo->grouphead->next) {
                                        //大群内一个人都没有
                                        Connect(tmpSocketInfo, SocketInfo);
                                    }
                                    else {
                                        //大群内已经有人
                                        JoinIn(tmpSocketInfo, SocketInfo);
                                    }
                                    strcpy_s(tmpstr, DATA_BUFSIZE, SocketInfo->name);
                                    strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                    _itoa_s(SocketInfo->account, tmpstr2, DATA_BUFSIZE, 10);
                                    strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                    strcat_s(tmpstr, DATA_BUFSIZE, ")---");
                                    strcat_s(tmpstr, DATA_BUFSIZE, "加入群聊！\n");
                                    strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                    if (isfound == 2) {
                                        strcpy_s(tmpstr, DATA_BUFSIZE, "你已退出当前群聊！\n");
                                        strcat_s(tmpstr, DATA_BUFSIZE, SocketInfo->DataBuf.buf);
                                        strcpy_s(SocketInfo->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                    }
                                    tmphere = SocketInfo->grouphead->next;
                                    while (tmphere) {
                                        //为群聊内的每一位成员发送消息
                                        strcpy_s(tmphere->pmember->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                        tmphere->pmember->BytesRECV = strlen(tmphere->pmember->DataBuf.buf);
                                        tmphere = tmphere->next;
                                    }
                                    sendcase = 1;
                                    //进入聊天间
                                    SocketInfo->command = 5;
                                    break;
                            }
                            switch (sendcase) {
                                case 1:
                                    //不做任何处理
                                    break;
                                case 2:
                                    tmphere = SocketInfo->grouphead->next;
                                    strcpy_s(NAME, NAME_SIZE, SocketInfo->name);
                                    strcat_s(tmpstr, DATA_BUFSIZE, NAME);
                                    strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                    _itoa_s(tmphere->pmember->account, tmpstr2, DATA_BUFSIZE, 10);
                                    strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                    strcat_s(tmpstr, DATA_BUFSIZE, "):\n");
                                    strcat_s(tmpstr, DATA_BUFSIZE, SocketInfo->DataBuf.buf);
                                    while (tmphere) {
                                        //为群聊内的每一位成员发送消息
                                        strcpy_s(tmphere->pmember->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                        tmphere->pmember->BytesRECV = strlen(tmphere->pmember->DataBuf.buf);
                                        //让这些成员把这些信息仅发给自己
                                        tmphere = tmphere->next;
                                    }
                                    break;
                                case 3:
                                    tmphere = SocketInfo->grouphead->next;
                                    strcpy_s(tmpstr, DATA_BUFSIZE, SocketInfo->name);
                                    strcat_s(tmpstr, DATA_BUFSIZE, "(");
                                    _itoa_s(SocketInfo->account, tmpstr2, DATA_BUFSIZE, 10);
                                    strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
                                    strcat_s(tmpstr, DATA_BUFSIZE, "):\n");
                                    strcat_s(tmpstr, DATA_BUFSIZE, SocketInfo->DataBuf.buf);
                                    strcat_s(tmpstr, DATA_BUFSIZE, "\n");
                                    printf("开始发送消息");
                                    while (NULL != tmphere) {
                                        //为群聊内的除了自己以外的每一位成员发送消息
                                        if (tmphere->pmember != SocketInfo) {
                                            printf("*");
                                            strcpy_s(tmphere->pmember->DataBuf.buf, DATA_BUFSIZE, tmpstr);
                                            tmphere->pmember->BytesRECV = strlen(tmphere->pmember->DataBuf.buf);
                                            //让这些成员把这些信息仅发给自己
                                        }
                                        tmphere = tmphere->next;
                                    }
                                    printf(SocketInfo->DataBuf.buf);
                                    ZeroMemory(SocketInfo->DataBuf.buf, DATA_BUFSIZE);      // 初始化缓冲区
                                    SocketInfo->BytesRECV = SocketInfo->DataBuf.len = 0;    
                                    break;
                            }
                            SocketInfo->BytesRECV = strlen(SocketInfo->DataBuf.buf);      // 记录接收数据的字节数                           
                            if (RecvBytes == 0)                                  // 如果接收到0个字节，则表示对方关闭连接
                            {
                                FreeSocketInformation(i);
                                continue;
                            }
                            else                                                            // 如果成功接收数据，则打印收到的数据
                            {
                                printf("\n");
                            }
                        }
                    }
                }
            }
            else
            {
                // 如果当前套接字在WriteSet集合中，则表明该套接字的内部数据缓冲区中有数据可以发送
                if (FD_ISSET(SocketInfo->Socket, &WriteSet))
                {
                    Total--;            // 减少一个处于就绪状态的套接字
                    SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;           // 初始化缓冲区位置
                    SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;    // 初始化缓冲区长度
                    if (SocketInfo->DataBuf.len > 0)     // 如果有需要发送的数据，则发送数据
                    {
                        if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
                            NULL, NULL) == SOCKET_ERROR)
                        {
                            // 错误编码等于WSAEWOULDBLOCK表示暂没有数据，否则表示出现异常
                            if (WSAGetLastError() != WSAEWOULDBLOCK)
                            {
                                printf("WSASend()   failed   with   error   %d\n", WSAGetLastError());
                                FreeSocketInformation(i);       // 释放套接字信息
                            }
                            continue;
                        }
                        else
                        {
                            SocketInfo->BytesSEND += SendBytes;         // 记录发送数据的字节数
                            // 如果从客户端接收到的数据都已经发回到客户端，则将发送和接收的字节数量设置为0
                            if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
                            {
                                SocketInfo->BytesSEND = 0;
                                SocketInfo->BytesRECV = 0;
                            }
                        }
                        ZeroMemory(SocketInfo->DataBuf.buf, DATA_BUFSIZE);// 初始化缓冲区
                        SocketInfo->BytesRECV = SocketInfo->DataBuf.len = 0;
                    }
                }

            }   // 如果ListenSocket未就绪，并且返回的错误不是WSAEWOULDBLOCK（该错误表示没有接收的连接请求），则出现异常

        }

    }
    // 暂停，按任意键退出
    system("pause");
    return 0;
}


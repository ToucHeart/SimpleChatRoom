#include <iostream>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <csignal>
#include <sys/ioctl.h>
#include <iomanip>
#include "helper.h"

using namespace std;

int serverfd;                 // 客户端socket
const char *IP = "127.0.0.1"; // 服务器的IP
short PORT = 10222;           // 服务器服务端口
typedef struct sockaddr SA;
char name[30];

size_t terminal_width() // 得到屏幕宽度
{
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col / 3;
};

void upRightPrint(bool up)
{
    if (up)
        cout << UPONELINE;                                     // 将光标向上移动一行
    cout << setiosflags(ios::right) << setw(terminal_width()); // 靠屏幕右侧1/3处输出
}

void initClient() // 初始化客户端
{
    serverfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(serverfd, (SA *)&addr, sizeof(addr)) == -1)
    {
        perror("无法连接到服务器");
        exit(-1);
    }
    char temp[32] = {0};
    if (recv(serverfd, temp, sizeof(temp), 0))
    {
        if (!strcmp(temp, "客户端启动成功"))
        {
            printStrs(1, 3, GREEN, temp, NORMAL);
        }
        else
        {
            printStrs(1, 3, RED, temp, NORMAL);
            exit(1);
        }
    }
}

void *recv_thread(void *p) // 客户端接收线程
{
    while (1)
    {
        char buf[100] = {};
        if (recv(serverfd, buf, sizeof(buf), 0) <= 0)
        {
            return NULL;
        }
        int pos = string(buf).find(':');
        if (pos != string::npos)
        {
            buf[pos] = '\0';
            printStrs(1, 4, GREEN, buf, ":", NORMAL);
            printStrs(2, 3, RED, buf + pos + 1, NORMAL);
        }
    }
}

void printSelfMsg(char *buf) // 打印自己的消息,先将光标向上移动一行,然后靠右侧输出
{
    upRightPrint(true);
    printStrs(1, 4, GREEN, name, ":", NORMAL);
    upRightPrint(false);
    printStrs(2, 3, RED, buf, NORMAL);
}

void start()
{
    pthread_t id;
    pthread_create(&id, NULL, recv_thread, NULL);
    char buf2[100] = {};
    sprintf(buf2, "%s:进入了聊天室", name);
    send(serverfd, buf2, strlen(buf2), 0);
    while (1)
    {
        char buf[100] = {};
        scanf("%s", buf);
        char msg[131] = {};
        sprintf(msg, "%s:%s", name, buf);
        printSelfMsg(buf);
        if (strcmp(buf, "bye") == 0)
        {
            memset(buf2, 0, sizeof(buf2));
            sprintf(buf2, "%s:退出了聊天室", name);
            send(serverfd, buf2, strlen(buf2), 0);
            break;
        }
        else
        {
            send(serverfd, msg, strlen(msg), 0);
        }
    }
    close(serverfd);
}

void getUserName() // 获取用户名,并判断是否重复
{
    printStrs(0, 1, "请输入您的名字：");
    while (1)
    {
        scanf("%s", name);
        send(serverfd, name, strlen(name), 0);
        char buf;
        read(serverfd, &buf, 1);
        if (buf == 0)
        {
            printStrs(0, 3, RED, "用户名已存在,请重新输入: ", NORMAL);
        }
        else
        {
            printStrs(1, 7, GREEN, "欢迎您! ", RED, name, GREEN, " 正在进入聊天室...", NORMAL);
            sleep(2);
            system("clear");
            break;
        }
    }
}

int main()
{
    initClient();
    getUserName();
    start();
    return 0;
}

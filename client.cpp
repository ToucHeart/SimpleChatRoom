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

size_t terminal_width()
{
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col / 3;
};

void upRightPrint(bool up)
{
    if (up)
        cout << UPONELINE;
    cout << setiosflags(ios::right) << setw(terminal_width());
}

void initClient()
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
        if (!strcmp(temp, "对不起，聊天室已满!"))
        {
            cout << RED << temp << NORMAL << endl;
            exit(1);
        }
        else
        {
            cout << GREEN << temp << NORMAL << endl;
        }
    }
}

void *recv_thread(void *p)
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
            cout << GREEN << buf << ":" << NORMAL << endl;
            cout << RED << buf + pos + 1 << NORMAL << endl
                 << endl;
        }
    }
}

void printSelfMsg(char *buf)
{
    upRightPrint(true);
    cout << GREEN << name << ":" << NORMAL << endl;
    upRightPrint(false);
    cout << RED << buf << NORMAL << endl
         << endl;
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

void getUserName()
{
    cout << "请输入您的名字：";
    while (1)
    {
        scanf("%s", name);
        send(serverfd, name, strlen(name), 0);
        char buf;
        read(serverfd, &buf, 1);
        if (buf == 0)
        {
            cout << RED "用户名已存在,请重新输入: " NORMAL;
        }
        else
        {
            cout << GREEN << "欢迎您! " << RED << name << NORMAL << endl;
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

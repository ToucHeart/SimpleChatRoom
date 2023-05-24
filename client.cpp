/*客户端的代码 在本目录……/SimpleChatRoom$下终端输入命令 ./client 即可运行,注意要先运行 ./server 开启服务器 */
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

void initClient() // 初始化客户端，在没有先运行服务器端时报错并退出。
{
    serverfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(serverfd, (SA *)&addr, sizeof(addr)) == -1)
    {
        perror("无法连接到服务器");//未运行服务器端
        exit(-1);//直接退出
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
        int pos = string(buf).find(':');//找到冒号的位置
        if (pos != string::npos)
        {
            buf[pos] = '\0';
            printStrs(1, 4, GREEN, buf, ":", NORMAL);//输出格式为：用户名:
                                                    //              消息
            printStrs(2, 3, RED, buf + pos + 1, NORMAL);
        }
    }
}

void printSelfMsg(char *buf) // 打印自己的消息,先将光标向上移动一行,然后靠右侧输出
{
    upRightPrint(true);
    printStrs(1, 4, GREEN, name, ":", NORMAL);//输出格式为：自己的用户名:
                                              //                   消息
    upRightPrint(false);
    printStrs(2, 3, RED, buf, NORMAL);
}

void start()
{
    pthread_t id;
    pthread_create(&id, NULL, recv_thread, NULL);//创建接收线程
    char buf2[100] = {};
    sprintf(buf2, "%s:进入了聊天室", name);//格式为：用户名:进入了聊天室
    send(serverfd, buf2, strlen(buf2), 0);
    while (1)
    {
        char buf[100] = {};
        scanf("%s", buf);//输入聊天内容
        char msg[131] = {};
        sprintf(msg, "%s:%s", name, buf);
        printSelfMsg(buf);//打印自己的消息
        if (strcmp(buf, "bye") == 0)//如果输入bye，则退出聊天室
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
        scanf("%s", name);//输入客户端用户名。
        send(serverfd, name, strlen(name), 0);//将名字发送给服务器端
        char buf;
        read(serverfd, &buf, 1);//接收服务器端的返回值
        if (buf == 0)
        {
            printStrs(0, 3, RED, "用户名已存在,请重新输入: ", NORMAL);
        }
        else
        {
            printStrs(1, 7, GREEN, "欢迎您! ", RED, name, GREEN, " 正在进入聊天室...", NORMAL);
            sleep(2);
            system("clear");//清屏,进入聊天室。
            break;
        }
    }
}

int main()
{
    initClient();//初始化客户端，在没有先运行服务器端时报错并退出。
    getUserName();// 获取用户名,并判断是否重复
    start();//正式进入聊天室,输入bye可以退出聊天室。
    return 0;
}

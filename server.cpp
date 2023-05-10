#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <map>
#include <string>
#include <csignal>
#include "helper.h"

using namespace std;

int serverFd; // 服务器socket
const int MAX_SIZE = 100;
const char *IP = "127.0.0.1";
const short PORT = 10222;
map<string, int> names_fd; // username,fd
typedef struct sockaddr SA;

void initServer() // 初始化服务器端
{
    serverFd = socket(PF_INET, SOCK_STREAM, 0);
    if (serverFd == -1)
    {
        perror("创建socket失败");
        exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (bind(serverFd, (SA *)&addr, sizeof(addr)) == -1)
    {
        perror("绑定失败");
        exit(-1);
    }
    if (listen(serverFd, 100) == -1)
    {
        perror("设置监听失败");
        exit(-1);
    }
}

void sendMsgToOthers(char *msg, int clientfd) // 向除了发送端以外的其他人发送
{
    for (auto itr = names_fd.begin(); itr != names_fd.end(); ++itr)
    {
        if (clientfd != (*itr).second)
        {
            printStrs(1, 4, "send to user: ", GREEN, (*itr).first.c_str(), NORMAL);
            send((*itr).second, msg, strlen(msg), 0);
        }
    }
}

bool checkNameExist(int clientfd) // 判断名字是否重复,重复发送0,不重复发送1
{
    char namebuf[30] = {};
    recv(clientfd, namebuf, sizeof(namebuf), 0);
    string requestName = string(namebuf);
    char rep = 0;
    if (names_fd.find(requestName) != names_fd.end())
    {
        send(clientfd, &rep, 1, 0);
        return false;
    }
    else
    {
        names_fd[requestName] = clientfd; // 添加到names_fd中
        rep = 1;
        send(clientfd, &rep, 1, 0);
    }
    return true;
}

void deleteUser(int fd) // 删除某个用户
{
    for (auto it = names_fd.begin(); it != names_fd.end(); it++)
    {
        if ((*it).second == fd)
        {
            names_fd.erase(it);
            return;
        }
    }
}

void *service_thread(void *p)
{
    int clientfd = *(int *)p;
    cout << "pthread = " << clientfd << endl;
    while (!checkNameExist(clientfd))
        ;
    while (1)
    {
        char buf[100] = {};
        if (recv(clientfd, buf, sizeof(buf), 0) <= 0)
        {
            // 找到username并删除
            deleteUser(clientfd);
            close(clientfd);
            cout << "fd = " << clientfd << "退出" << endl;
            pthread_exit(NULL);
        }
        // 把服务器接受到的信息发给所有的客户端
        sendMsgToOthers(buf, clientfd);
    }
}

void startService()
{
    cout << "服务器启动..." << endl;
    while (1)
    {
        struct sockaddr_in fromaddr;
        socklen_t len = sizeof(fromaddr);
        int clientfd = accept(serverFd, (SA *)&fromaddr, &len);
        if (clientfd == -1)
        {
            cout << "客户端连接出错..."
                 << endl;
            continue;
        }
        if (names_fd.size() >= MAX_SIZE)
        {
            const char *str = "对不起，聊天室已满!";
            send(clientfd, str, strlen(str), 0);
            close(clientfd);
        }
        else
        {
            cout << "连接到客户端,fd = " << clientfd << endl;
            const char *str = "客户端启动成功";
            send(clientfd, str, strlen(str), 0);
            // 有客户端连接之后，启动线程给此客户服务
            pthread_t tid;
            pthread_create(&tid, 0, service_thread, &clientfd);
        }
    }
}

void signal_handler(int sig)
{
    close(serverFd);
    exit(0);
}

int main()
{
    initServer();
    signal(SIGINT, signal_handler); // 捕获ctrl+c信号
    startService();
}

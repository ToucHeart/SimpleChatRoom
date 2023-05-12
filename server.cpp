/*服务器端的代码 在本目录……/SimpleChatRoom$下终端输入命令 ./server 开启服务器 */
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
map<string, int> names_fd; // username,fd 键值对。
typedef struct sockaddr SA;

void initServer() // 初始化服务器端
{
    serverFd = socket(PF_INET, SOCK_STREAM, 0);//创建socket 返回文件描述符。
    if (serverFd == -1)
    {
        perror("创建socket失败");
        exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (bind(serverFd, (SA *)&addr, sizeof(addr)) == -1)//绑定
    {
        perror("绑定失败");
        exit(-1);
    }
    if (listen(serverFd, 100) == -1)//设置监听
    {
        perror("设置监听失败");
        exit(-1);
    }
}

void sendMsgToOthers(char *msg, int clientfd) // 向除了发送端以外的其他人发送
{
    for (auto itr = names_fd.begin(); itr != names_fd.end(); ++itr)//遍历names_fd
    {
        if (clientfd != (*itr).second)//如果不是发送端都发送。
        {
            printStrs(1, 4, "send to user: ", GREEN, (*itr).first.c_str(), NORMAL);//服务器端打印发送给谁的记录。
            send((*itr).second, msg, strlen(msg), 0);
        }
    }
}

bool checkNameExist(int clientfd) // 判断名字是否重复,重复发送0,不重复发送1
{
    char namebuf[30] = {};
    recv(clientfd, namebuf, sizeof(namebuf), 0);//接收用户名
    string requestName = string(namebuf);
    char rep = 0;
    if (names_fd.find(requestName) != names_fd.end())
    {//如果用户名已经存在就返回失败。
        send(clientfd, &rep, 1, 0);
        return false;
    }
    else
    {
        names_fd[requestName] = clientfd; // 不存在则添加到names_fd中并返回成功。
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
            names_fd.erase(it);//遍历到就删。
            return;
        }
    }
}

void *service_thread(void *p)
{
    int clientfd = *(int *)p;
    cout << "pthread = " << clientfd << endl;
    while (!checkNameExist(clientfd))//服务线程，检查用户名是否重复。
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
        if (names_fd.size() >= MAX_SIZE)//多于100个客户端就拒绝连接。
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
    initServer(); // 初始化服务器端
    signal(SIGINT, signal_handler); // 捕获ctrl+c信号，输入ctrl+c退出服务器
    startService();//服务器启动。准备接入线程。
}

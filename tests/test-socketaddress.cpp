#include "../src/SocketAddress.hpp"
#include <iostream>
#include <unistd.h>

int main()
{
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 端口复用
    int opt =1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 绑定地址
    SocketAddress sockhelper("0.0.0.0", 8888);
    bind(sockfd, &sockhelper, sockhelper.AddrSize());
    // 监听套接字
    listen(sockfd, 3);
    // 等待连接
    SocketAddress client;
    socklen_t len = client.AddrSize();
    int newfd = accept(sockfd, &client, &len);
    client.ToHost();
    // 打印连接的客户端的信息
    std::cout << "ip = " << client.GetIP() << ", port = " << client.GetPort() << std::endl;

    while (1)
    {
        char buffer[1024];
        int n = recv(newfd, buffer, 1024, 0);
        buffer[n] = '\0';
        std::cout << "buffer = " << buffer << std::endl;
    }

    close(newfd);

    return 0;
}
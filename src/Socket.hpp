/*
 * CopyRight Limuyou
 * Date 24_11_5
 * brief 实现了对套接字接口功能的封装，便于使用
 */

#pragma once
#include "SocketAddress.hpp"
#include <spdlog/spdlog.h>

class Socket;

const int DefaultFd = -1;
const int MaxBackLog = 1024;
const int DefaultBufferSize = 1024;
using SockPtr = std::shared_ptr<Socket>;

class Socket
{
public:
    virtual void CreateSocket() = 0;                                           // 创建套接字文件
    virtual void ReusePort() = 0;                                              // 地址复用
    virtual bool SetNonBlock(int fd) = 0;                                      // 设置为非阻塞套接字
    virtual void BindAddress(uint16_t, const std::string &ip = "0.0.0.0") = 0; // 绑定地址
    virtual void CreateListen(int backlog = MaxBackLog) = 0;                   // 监听连接
    virtual void Connect(const std::string &, uint16_t) = 0;                   // 连接
    virtual int AcceptConnect(SocketAddress &, int &) = 0;                     // 接受连接
    virtual ssize_t Recv(std::string &out) = 0;                                // 接收消息
    virtual ssize_t Send(const std::string &) = 0;                             // 发送消息
    virtual int GetFd() = 0;                                                   // 获取文件描述符

public:
    void BuildTcpListen(uint16_t PORT, const std::string &IP = "0.0.0.0")
    {
        CreateSocket();

        if (!SetNonBlock(GetFd()))
        {
            spdlog::error("Set Listenfd NonBlock Error...");
            exit(-1);
        };
        ReusePort();
        BindAddress(PORT, IP);
        CreateListen();
    }

    void BuildTcpClient(const std::string &IP, uint16_t PORT)
    {
        CreateSocket();
        Connect(IP, PORT);
    }
};

class TcpSocket : public Socket
{
public:
    TcpSocket() = default;

    TcpSocket(int fd)
        : _sockfd(fd)
    {
    }

    void CreateSocket()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            perror("socket:");
            exit(1);
        }

        spdlog::debug("Successful create socket = {}...", _sockfd);
    }

    void ReusePort()
    {
        int opt = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    bool SetNonBlock(int fd) override
    {
        // 获取现在文件描述符的状态，添加非阻塞, 写回
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 1)
        {
            spdlog::error("fcntl F_GETFL error...");
            return false;
        }

        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            spdlog::error("fcntl F_SETFL error...");
            return false;
        }

        return true;
    }

    void BindAddress(uint16_t port, const std::string &ip) override
    {
        SocketAddress sockhelper(ip, port);
        if (bind(_sockfd, &sockhelper, sockhelper.AddrSize()) < 0)
        {
            perror("bind:");
            exit(1);
        }

        spdlog::debug("Successful bind address...");
    }

    void CreateListen(int backlog) override
    {
        if (listen(_sockfd, backlog) < 0)
        {
            perror("listen");
            exit(-1);
        }

        spdlog::debug("Successful listening...");
    }

    int AcceptConnect(SocketAddress &client, int &code) override
    {
        // struct sockaddr_in client;
        // socklen_t len = sizeof(client);

        errno = 0;
        socklen_t len = client.AddrSize();
        int fd = accept(_sockfd, &client, &len);
        client.ToHost(); // 初始化一下信息
        code = errno;

        if (fd < 0)
        {
            perror("accept");
            return fd;
        }
        spdlog::debug("Successful accept...");

        if (!SetNonBlock(fd))
        {
            spdlog::error("Set fd nonblock error...");
            return -1;
        }

        return fd;
    }

    void Connect(const std::string &ip, uint16_t port) override
    {
        SocketAddress server(ip, port);
        socklen_t len = server.AddrSize();

        int n = connect(_sockfd, &server, len);
        if (n < 0)
        {
            perror("connect");
            exit(-1);
        }

        spdlog::debug("Successful connect...");
    }

    ssize_t Recv(std::string &out) override
    {
        char OutBuff[DefaultBufferSize];
        ssize_t n = recv(_sockfd, OutBuff, sizeof(OutBuff) - 1, 0);

        if (n > 0)
        {
            // 收到数据，将其写入 out，并返回接收到的字节数
            OutBuff[n] = '\0';
            out.assign(OutBuff, n); 
            spdlog::debug("Successful recv msg: {}", out);
            return n;
        }
        else if (n == 0)
        {
            // n == 0 表示对方关闭连接
            spdlog::info("Connection closed by peer.");
            return -1;
        }
        else
        {
            // n < 0，检查 errno
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 非阻塞模式下没有数据可读
                spdlog::debug("No data available (EAGAIN/EWOULDBLOCK).");
                return 0;
            }
            else if (errno == EINTR)
            {
                // 调用被信号中断，可以选择重试
                spdlog::warn("recv interrupted by signal (EINTR).");
                return 1; // 返回 0 表示暂时无数据
            }
            else
            {
                // 其他错误
                spdlog::error("recv error: {}", strerror(errno));
                return -1; // 返回 -1 表示真正的错误
            }
        }
    }

    ssize_t Send(const std::string &in) override
    {
        int n = send(_sockfd, in.c_str(), in.size(), 0);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
            {
                return 0;
            }
            perror("send");
            return -1;
        }

        spdlog::info("Successful send msg...");
        return n;
    }

    int GetFd() override
    {
        return _sockfd;
    }

private:
    int _sockfd;
};

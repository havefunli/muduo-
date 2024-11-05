#pragma once
#include "SocketAddress.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <cstdio>
#include <iostream>
#include <string>
#include <spdlog/spdlog.h>

class Socket;

const int DefaultFd = -1;
const int DefaultBackLog = 5;
const int DefaultBufferSize = 1024;
using SockPtr = std::shared_ptr<Socket>;

class Socket
{
public:
    virtual void CreateSocket() = 0; // 创建套接字文件
    virtual bool SetNonBlock(int fd) = 0;  // 设置为非阻塞套接字
    virtual void BindAddress(uint16_t, const std::string& ip = "0.0.0.0") = 0; // 绑定地址
    virtual void CreateListen(int backlog = DefaultBackLog) = 0; // 监听连接
    virtual void Connect(const std::string&, uint16_t) = 0; // 连接
    virtual int AcceptConnect(SocketAddress&, int&) = 0; // 接受连接
    virtual ssize_t Recv(std::string& out) = 0; // 接收消息
    virtual ssize_t Send(const std::string&) = 0; // 发送消息
    virtual int GetFd() = 0; // 获取文件描述符

public:
    void BuildTcpListen(uint16_t PORT, const std::string& IP = "0.0.0.0")
    {
        CreateSocket();
        
        if (!SetNonBlock(GetFd())) 
        {
            spdlog::error("Set Listenfd NonBlock Error...");
            exit(-1);
        };

        BindAddress(PORT, IP);
        CreateListen();
    }

    void BuildTcpClient(const std::string& IP, uint16_t PORT)
    {
        CreateSocket();
        Connect(IP, PORT);
    }
};


class TcpSocket : public Socket
{
public: 
    TcpSocket() {}

    TcpSocket(int fd) 
        : _sockfd(fd)  
    {}

    void CreateSocket()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(_sockfd < 0) 
        {
            perror("socket:");
            exit(1);
        }

        spdlog::info("Successful create socket = {}...", _sockfd);
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

    void BindAddress(uint16_t port, const std::string& ip) override
    {
        SocketAddress sockhelper(ip, port);
        if(bind(_sockfd, &sockhelper, sockhelper.AddrSize()) < 0) 
        {
            perror("bind:");
            exit(1);
        }

        spdlog::info("Successful bind address...");
    }
    
    void CreateListen(int backlog) override
    {
        if(listen(_sockfd, backlog) < 0) 
        {
            perror("listen");
            exit(-1);
        }

        spdlog::info("Successful listening...");
    }

    int AcceptConnect(SocketAddress& client, int& code) override
    {
        // struct sockaddr_in client;
        // socklen_t len = sizeof(client);

        socklen_t len = client.AddrSize();

        errno = 0;
        int fd = accept(_sockfd, &client, &len);
        code = errno;

        if(fd < 0) 
        {
            perror("accept");
            return fd;
        }
        spdlog::info("Successful accept..." );

        if (!SetNonBlock(fd))
        {
            std::cerr << "Set fd nonblock error..." << std::endl;
            return -1;
        }
        
        return fd;
    }

    void Connect(const std::string& ip, uint16_t port) override
    {
        SocketAddress server(ip, port);
        socklen_t len = server.AddrSize();

        int n = connect(_sockfd, &server, len);
        if(n < 0)
        {
            perror("connect");
            exit(-1);
        }

        spdlog::info("Successful connect...");
    }

    ssize_t Recv(std::string& out) override
    {
        char OutBuff[DefaultBufferSize];
        ssize_t n = recv(_sockfd, OutBuff, sizeof(OutBuff) - 1, 0);
        if(n > 0)
        {
            OutBuff[n] = '0';
            out = OutBuff;
            spdlog::info("Successful recv msg...");
            return n;
        }
        else if(n < 0)
        {
            perror("recv:");
            return n;
        }
        else
        {
            return n;
        }
    }

    ssize_t Send(const std::string& in) override
    {
        int n = send(_sockfd, in.c_str(), in.size(), 0);
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

#include "../include/Epoll.h"
#include "../src/Socket.hpp"
#include <iostream>

void HandleClose(Channel* channel)
{
    channel->ReMove();
    delete channel;
}

void HandleRead(Channel* channel)
{
    TcpSocket conn(channel->GetFd());
    std::string buff;
    int n = conn.Recv(buff);
    if (n <= 0)
    {
        channel->ReMove();
        return;
    }
    spdlog::info("The fd = {}: {}", channel->GetFd(), buff);
    channel->EnableWriteable(true);
}

void HandleWrite(Channel* channel)
{
    TcpSocket conn(channel->GetFd());
    int n = conn.Send("你好呀！\n");
    if (n <= 0)
    {
        channel->ReMove();
        return;
    }
    channel->EnableWriteable(false);
}

void HandleExcept(Channel* channel)
{
    channel->ReMove();
}

void HandleEvent(Channel* channel)
{
    std::cout << "处理了一个事件..." << std::endl;
}

void Accept(std::shared_ptr<Epoll> epoll_ptr, Channel* c1)
{
    TcpSocket tcp_server(c1->GetFd());
    int code = 0;
    SocketAddress client;
    int fd = tcp_server.AcceptConnect(client, code);
    
    Channel* c2 = new Channel(fd);
    c2->SetEventLoop(epoll_ptr);
    c2->RigisterEventsFunc(std::bind(HandleRead, c2), 
                           std::bind(HandleWrite, c2),
                           std::bind(HandleExcept, c2));
    c2->SetCloseFunc(std::bind(HandleClose, c2));
    c2->SetEventCallBack(std::bind(HandleEvent, c2));

    c2->EnableReadable(true);
}   


int main()
{
    spdlog::set_level(spdlog::level::debug);
    std::unique_ptr<Socket> sock_ptr = std::make_unique<TcpSocket>();
    sock_ptr->BuildTcpListen(8888);

    EpollPtr epoll_ptr = std::make_shared<Epoll>();
    Channel channel(sock_ptr->GetFd());
    channel.SetEpoll(epoll_ptr);
    channel.RigisterEventsFunc(std::bind(Accept, epoll_ptr, &channel), 0, 0);
    channel.EnableReadable(true); 

    while (1)
    {
        std::vector<Channel*> active;
        epoll_ptr->Wait(active);
        for (auto& event : active)
        {
            event->HandleEvent();
        }
    }
    
    return 0;
}

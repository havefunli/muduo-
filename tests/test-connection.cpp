#include "../include/Connection.h"
#include <iostream>

int id = 1;
std::unordered_map<int, ConnPtr> connections;

void Handle_Connected(ConnPtr conn)
{
    std::cout << "Successful connected, fd = " << conn->GetFd() << std::endl;
}

void Handle_Msg(ConnPtr conn, BufferPtr buf)
{
    std::string msg = buf->ReadAllContent();
    std::cout << "Msg = " << msg;

    std::string ret = "处理好了！\n";
    conn->Send(ret.c_str(), ret.size());
}

void Handle_Closed(ConnPtr conn)
{
    connections.erase(conn->GetId());
}

void Accept(EveLoopPtr loop, ChannelPtr channel)
{
    int newfd = accept(channel->GetFd(), nullptr, nullptr);
    if (newfd < 0) return;

    ConnPtr conn = std::make_shared<Connection>(id, newfd, loop);
    conn->SetConnectedCallBack(std::bind(Handle_Connected, std::placeholders::_1));
    conn->SetMessageCallBack(std::bind(Handle_Msg, std::placeholders::_1, std::placeholders::_2));
    conn->SetClosedCallBack(std::bind(Handle_Closed, std::placeholders::_1));
    conn->EnableMonitorActivity(true);
    conn->Established();
    
    connections.insert(std::make_pair(id, conn));
    id++;
}   


int main()
{
    spdlog::set_level(spdlog::level::debug);
    std::unique_ptr<Socket> sock_ptr = std::make_unique<TcpSocket>();
    sock_ptr->BuildTcpListen(8888);

    EveLoopPtr loop = std::make_shared<EventLoop>();
    ChannelPtr channel = std::make_shared<Channel>(sock_ptr->GetFd());
    channel->SetEventLoop(loop.get());
    channel->RigisterEventsFunc(std::bind(Accept, loop, channel), nullptr, nullptr);
    channel->EnableReadable(true);

    while (1)
    {
        loop->Start();
    }
    
    return 0;
}
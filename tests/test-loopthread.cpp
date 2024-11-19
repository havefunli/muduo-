#include "../include/Acceptor.h"
#include "../include/Connection.h"
#include "../include/LoopThread.h"
#include <iostream>

int id = 1;
std::unordered_map<int, ConnPtr> connections;

EveLoopPtr base_loop = std::make_shared<EventLoop>();
LoopThreadPool thread_pool(2, base_loop);


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
    // int n = connections.erase(conn->GetId()); // 在这里真的删除了，后续操作就出错了
    std::cout << "删除一下..." << std::endl;
}

void Accept(int newfd)
{
    ConnPtr conn = std::make_shared<Connection>(id, newfd, thread_pool.NextLoop());

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

    Acceptor acceptor(8888, base_loop);
    acceptor.SetAcceptCallBack(std::bind(Accept, std::placeholders::_1));
    acceptor.Listen();

    thread_pool.InitPool();

    while (1) { base_loop->Start(); }
    
    return 0;
}
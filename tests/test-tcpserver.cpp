#include "../include/TcpServer.h"

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
    std::cout << "Successful disconnected..." << std::endl;
}

int main()
{
    spdlog::set_level(spdlog::level::debug);
    
    TcpServer server(8888, 2);
    server.SetConnectedCallBack(Handle_Connected);
    server.SetMessageCallBack(Handle_Msg);
    server.SetClosedCallBack(Handle_Closed);
    server.EnableMonitorActivity(true);
    server.Start();

    return 0;
}
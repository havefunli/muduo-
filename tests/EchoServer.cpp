#include "../include/TcpServer.h"

class EchoServer
{
private:
    void Handle_Connected(ConnPtr conn)
    {
        std::cout << "Successful connected, fd = " << conn->GetFd() << std::endl;
    }

    void Handle_Msg(ConnPtr conn, BufferPtr buf)
    {
        std::string msg = buf->ReadAllContent();
        std::cout << "Msg:" << msg << std::endl;
        conn->Send(msg.c_str(), msg.size());
    }

    void Handle_Closed(ConnPtr conn)
    {
        std::cout << "Successful disconnected..." << std::endl;
    }

public:
    EchoServer(int port, int thread_num)
        : _server(port, thread_num)
    {
        _server.EnableMonitorActivity(true);
        _server.SetConnectedCallBack(std::bind(&EchoServer::Handle_Connected, this, std::placeholders::_1));
        _server.SetMessageCallBack(std::bind(&EchoServer::Handle_Msg, this, std::placeholders::_1, std::placeholders::_2));
        _server.SetClosedCallBack(std::bind(&EchoServer::Handle_Closed, this, std::placeholders::_1));
    }

    void Start()
    {
        _server.Start();
    }

private:
    TcpServer _server;
};


int main()
{
    EchoServer server(8888, 5);
    server.Start();
    return 0;
}
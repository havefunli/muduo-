#include "../include/Acceptor.h"

Acceptor::Acceptor(uint16_t port, EveLoopPtr loop)
    : _socket(std::make_shared<TcpSocket>())
    , _loop(loop)
    , _channel(std::make_shared<Channel>())
{
    _socket->BuildTcpListen(port);
    _channel->SetFd(_socket->GetFd());
    _channel->SetEventLoop(_loop.get());
    _channel->RigisterEventsFunc(std::bind(&Acceptor::HandleRead, this), nullptr, nullptr);
    spdlog::info("Successful create Acceptor...");
}

Acceptor::~Acceptor() = default;

void Acceptor::SetAcceptCallBack(const AcceptCallBack& cb)
{
    _accept_cb = cb;
}

void Acceptor::Listen()
{
    _channel->EnableReadable(true);
}

// ET 模式需要不断读取确认底层数据被取完
void Acceptor::HandleRead()
{
    while (true)
    {
        SocketAddress client;
        int code = 0;  // 接受的错误码
        int newfd = _socket->AcceptConnect(client, code);

        if (newfd < 0) 
        {
            if (code == EAGAIN || code == EWOULDBLOCK) 
            {
                // 无新连接，正常退出循环
                spdlog::info("No new connection (EAGAIN/EWOULDBLOCK). Exiting...");
                break;
            } 
            else 
            {
                // 其他错误码，记录日志并退出
                spdlog::error("AcceptConnect failed, error code = {}", code);
                break;
            }
        }

        spdlog::info("NewConnection: Client ip = {}, port = {}, newfd = {}...", 
                      client.GetIP().c_str(), client.GetPort(), newfd);

        if (_accept_cb) { _accept_cb(newfd); }
    }
}
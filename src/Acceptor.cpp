#include "../include/Acceptor.h"

Acceptor::Acceptor(uint16_t port, EveLoopPtr loop)
    : _socket(std::make_shared<TcpSocket>())
    , _loop(loop)
    , _channel(std::make_shared<Channel>())
{
    _socket->BuildTcpListen(port);
    _channel->SetFd(_socket->GetFd());
    _channel->SetEventLoop(_loop.get());
}

Acceptor::~Acceptor() = default;

void Acceptor::SetAcceptCallBack(const AcceptCallBack& cb)
{
    _accept_cb = cb;
}

void Acceptor::HandleRead()
{
    SocketAddress client;
    int code = 0;
    int newfd = _socket->AcceptConnect(client, code);

    if (_accept_cb) { _accept_cb(newfd); }
}
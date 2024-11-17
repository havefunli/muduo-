#include "../src/Socket.hpp"
#include "EventLoop.h"

using AcceptCallBack = std::function<void(int)>;

class Acceptor
{
private:
    // 处理产生的新连接
    void HandleRead();

public:
    Acceptor(uint16_t, EveLoopPtr);

    ~Acceptor();

    void SetAcceptCallBack(const AcceptCallBack&);

private:
    SockPtr        _socket;    // 创建监听套接字
    EveLoopPtr     _loop;      // 监控套接字事件
    ChannelPtr     _channel;   // 管理套接字事件
    AcceptCallBack _accept_cb; // 设置连接回调函数
};
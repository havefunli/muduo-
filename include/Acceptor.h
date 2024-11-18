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

    // 这个用于开启事件监控
    // 不能直接在构造函数开启
    // 是因为如果新连接来了，但是并未设置连接回调函数
    // 就不能正常处理连接，所以必须在设置回调函数之后再打开监听
    void Listen();

    void SetAcceptCallBack(const AcceptCallBack&);

private:
    SockPtr        _socket;    // 创建监听套接字
    EveLoopPtr     _loop;      // 监控套接字事件
    ChannelPtr     _channel;   // 管理套接字事件
    AcceptCallBack _accept_cb; // 设置连接回调函数
};
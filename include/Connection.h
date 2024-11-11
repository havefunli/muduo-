#pragma once

#include "Channel.h"
#include "EventLoop.h"
#include "../src/Socket.hpp"
#include "../src/Any.hpp"
#include "../src/Buffer.hpp"

enum ConStatus
{
    CONNECTING,    // 正在连接
    CONNECTED,     // 已连接
    DISCONNECTING, // 正在断开
    DISCONNECTED   // 已断开
};

class Connection;
using ConnPtr = std::shared_ptr<Connection>;
const int DefaultTimeout = 60;

using ConnectedCallBack = std::function<void(ConnPtr)>;
using MessageCallBack   = std::function<void(ConnPtr)>;
using ClosedCallBack    = std::function<void(ConnPtr)>;
using AnyEventCallBack  = std::function<void(ConnPtr)>;

class Connection : public std::enable_shared_from_this<Connection>
{
private:
    // 描述符可读后调用的函数，将数据写入接受缓冲区
    void HandleRead();

    // 描述符可写调用的函数，将发送缓冲区数据发送
    void HandleWrite();

    // 描述符触发挂断事件
    void HandleClose();

    // 描述符触发错误事件
    void HandleError();

    // 出发任意事件后的时间调用：
    // - 刷新定时器任务
    // - 用户定义的任务
    void HandleEvent();

    // 在关闭连接前处理剩余数据
    void ShutDownInLoop();

    // 连接监控后，进行的设置，启动读监控，调用回调
    void EstablishedLoop();

    // 真正地关闭操作
    void ReleaseInLoop();

    void SendInLoop(char* data, size_t n);

    // 管理连接定时器任务
    void EnableTimerInLoop(bool, int sec);

public:
    Connection();
    
    ~Connection();
    
    // 发送数据，将数据写入发送缓冲区，打开写事件监控
    void Send(char* data, size_t n);

    // 关闭连接，但是首先需要判断数据是否处理完毕
    void CloseConn();

    // 是否启用活跃监控，并且定时时间(默认 1 min)
    void EnableMonitorActivity(bool, int sec = DefaultTimeout);
     

private:
    int        _sockfd;    // 管理的套接字
    uint16_t   _conn_id;   // 连接 ID，后续方便使用
    ConStatus  _status;    // 连接的状态
    SockPtr    _socket;    // 套接字的事件管理
    ChannelPtr _channel;   // 连接的事件管理
    EveLoopPtr _loop;      // 关联的 eventloop
    Buffer     _inbuf;     // 输入缓冲区
    Buffer     _outbuf;    // 输出缓冲区
    Any        _content;   // 请求的接受处理上下文
    bool       _istmract;  // 是否激活计时器

    ConnectedCallBack _conn_cb;
    MessageCallBack   _msg_cb;
    ClosedCallBack    _closed_cb;
    ClosedCallBack    _server_closed_cb;  // 服务器的关闭回调
    AnyEventCallBack  _any_cb;
};
#pragma once

#include "Channel.h"
#include "../src/Any.hpp"
#include "../src/Buffer.hpp"

class Connection;
using ConnPtr = std::shared_ptr<Connection>();

enum ConStatus
{

};

const int DefaultTimeout = 60;

using ConnectedCallBack = std::function<void(ConnPtr)>;
using MessageCallBack   = std::function<void(ConnPtr)>;
using ClosedCallBack    = std::function<void(ConnPtr)>;
using AnyEventCallBack  = std::function<void(ConnPtr)>;

class Connection
{
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
    int       _sockfd;   // 管理的套接字
    uint16_t  _conn_id;  // 连接 ID，后续方便使用
    ConStatus _status;   // 连接的状态
    Channel   _channel;  // 连接的事件管理
    Buffer    _inbuf;    // 输入缓冲区
    Buffer    _outbuf;   // 输出缓冲区
    Any       _content;  // 请求的接受处理上下文

    ConnectedCallBack _conn_cb;
    MessageCallBack   _msg_cb;
    ClosedCallBack    _closed_cb;
    AnyEventCallBack  _any_cb;
}
#include "../include/Connection.h"


void Connection::HandleRead()
{
    // 读取处理
    std::string buf;
    int n = _socket->Recv(buf);
    if (n < 0) { return ShutDownInLoop(); } // 读取出错
    // 在这里等于 0 不再是断开连接
    _inbuf.Write(buf.c_str(), buf.size()); // 写入缓冲区
    
    // 数据处理
    if (_inbuf.ReadableBytes())
    {
        _msg_cb(shared_from_this());
    }
}
 
void Connection::HandleWrite()
{
    int n = _socket->Send(_outbuf.ReadAllContent());
    // 发送事件异常
    if (n < 0) 
    {
        // 有无数据没处理, 有则处理
        if (_inbuf.ReadableBytes()) { _msg_cb(shared_from_this()); }
        ReleaseInLoop(); // 关闭操作
    }
    // 若没有数据处理了，关闭写事件
    if (_outbuf.ReadableBytes() == 0) { _channel->EnableWriteable(false); }
}

void Connection::HandleClose()
{
    // 查看是否剩余数据处理
    if (_inbuf.ReadableBytes()) { _msg_cb(shared_from_this()); }
    ReleaseInLoop(); // 关闭操作
}

void Connection::HandleError()
{
    HandleClose();
}

void Connection::HandleEvent()
{
    if (_istmract) { _loop->RefreshTimer(_conn_id); }
    if (_any_cb) { _any_cb(shared_from_this()); }
}

void Connection::EstablishedLoop()
{
    // 设置为连接状态
    _status = CONNECTED;
    _channel->EnableReadable(true);
    // 连接事件回调
    if (_conn_cb) { _conn_cb(shared_from_this()); }
}

void Connection::ReleaseInLoop()
{
    _status = DISCONNECTED;
    // 移除对事件的监控
    _channel->ReMove();
    // 关闭文件描述符
    close(_sockfd);
    // 移除定时器中的任务
    if (_loop->HasTimer(_conn_id)) { EnableTimerInLoop(false, 0); }
    // 调用用户关闭回调
    if (_closed_cb) { _closed_cb(shared_from_this()); }
    // 调用服务器的关闭回调
    if (_server_closed_cb) { _server_closed_cb(shared_from_this()); }
}

void Connection::SendInLoop(char* data, size_t n)
{
    // 将数据写入缓冲区
    _outbuf.Read(data, n);
    // 开启写监控，帮我们发送
    _channel->EnableWriteable(true);
}

void Connection::ShutDownInLoop()
{
    _status = CONNECTING;
    // 还有数据未处理
    if (_inbuf.ReadableBytes())
    {
        if (_msg_cb) { _msg_cb(shared_from_this()); }
    }
    // 查看还是否存在数据未发送
    if (_outbuf.ReadableBytes()) { _channel->EnableWriteable(true); }
    // 真正的关闭
    ReleaseInLoop();
}

void Connection::EnableTimerInLoop(bool isvalid, int timeout)
{   
    // 启用，存在就刷新，不存在就添加
    if (isvalid)
    {
        _istmract = true;
        if (_loop->HasTimer(_conn_id)) { return _loop->RefreshTimer(_conn_id); }
        else { _loop->AddTimer(_conn_id, timeout, std::bind(Connection::ReleaseInLoop, this)); }
    }
    else
    {
        _istmract = false;
        if (_loop->HasTimer(_conn_id)) { _loop->EnableTimer(_conn_id, false); }
    }
}
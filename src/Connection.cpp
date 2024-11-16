#include "../include/Connection.h"

Connection::Connection(int conn_id, int sockfd, EveLoopPtr loop)
    : _conn_id(conn_id)
    , _sockfd(sockfd)
    , _istmract(false)
    , _status(CONNECTING)
    , _loop(loop)
    , _inbuf(std::make_shared<Buffer>())
    , _outbuf(std::make_shared<Buffer>())
{
    _channel->SetEventLoop(loop.get());
    _channel->RigisterEventsFunc(std::bind(&Connection::HandleRead, this),
                                 std::bind(&Connection::HandleWrite, this),
                                 std::bind(&Connection::HandleError, this));
    _channel->SetCloseFunc(std::bind(&Connection::HandleClose, this));
    _channel->SetEventCallBack(std::bind(&Connection::HandleEvent, this));
}

Connection::~Connection() = default;

int Connection::GetFd()
{
    return _sockfd;
}

uint16_t Connection::GetId()
{
    return _conn_id;
} 

ConStatus Connection::GetStatus()
{
    return _status;
}

void Connection::HandleRead()
{
    // 读取处理
    std::string buf;
    int n = _socket->Recv(buf);
    if (n < 0) { return ShutDownInLoop(); } // 读取出错
    // 在这里等于 0 不再是断开连接
    _inbuf->Write(buf.c_str(), buf.size()); // 写入缓冲区
    
    // 数据处理
    if (_inbuf->ReadableBytes())
    {
        _msg_cb(shared_from_this(), _inbuf);
    }
}
 
void Connection::HandleWrite()
{
    int n = _socket->Send(_outbuf->ReadAllContent());
    // 发送事件异常
    if (n < 0) 
    {
        // 有无数据没处理, 有则处理
        if (_inbuf->ReadableBytes()) { _msg_cb(shared_from_this(), _inbuf); }
        ReleaseInLoop(); // 关闭操作
    }
    // 若没有数据处理了，关闭写事件
    if (_outbuf->ReadableBytes() == 0) { _channel->EnableWriteable(false); }
}

void Connection::HandleClose()
{
    // 查看是否剩余数据处理
    if (_inbuf->ReadableBytes()) { _msg_cb(shared_from_this(), _inbuf); }
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
    if (_svr_closed_cb) { _svr_closed_cb(shared_from_this()); }
}

void Connection::EstablishedLoop()
{
    // 设置为连接状态
    _status = CONNECTED;
    _channel->EnableReadable(true);
    // 连接事件回调
    if (_conn_cb) { _conn_cb(shared_from_this()); }
}

void Connection::Established()
{
    _loop->RunInLoop(std::bind(&Connection::EstablishedLoop, this));
}

void Connection::SendInLoop(const char* data, size_t n)
{
    // 将数据写入缓冲区
    _outbuf->Write(data, n);
    // 开启写监控，帮我们发送
    _channel->EnableWriteable(true);
}

void Connection::Send(const char* data, size_t n)
{
    _loop->RunInLoop(std::bind(&Connection::SendInLoop, this, data, n));
}

void Connection::ShutDownInLoop()
{
    _status = DISCONNECTING;
    // 还有数据未处理
    if (_inbuf->ReadableBytes())
    {
        if (_msg_cb) { _msg_cb(shared_from_this(), _inbuf); }
    }
    // 查看还是否存在数据未发送
    if (_outbuf->ReadableBytes()) { _channel->EnableWriteable(true); }
    // 真正的关闭
    ReleaseInLoop();
}

void Connection::ShutDown()
{
    _loop->RunInLoop(std::bind(&Connection::ShutDownInLoop, this));
}

void Connection::EnableTimerInLoop(bool isvalid, int timeout)
{   
    // 启用，存在就刷新，不存在就添加
    if (isvalid)
    {
        _istmract = true;
        if (_loop->HasTimer(_conn_id)) { return _loop->RefreshTimer(_conn_id); }
        else { _loop->AddTimer(_conn_id, timeout, std::bind(&Connection::ReleaseInLoop, this)); }
    }
    else
    {
        _istmract = false;
        if (_loop->HasTimer(_conn_id)) { _loop->EnableTimer(_conn_id, false); }
    }
}

void Connection::EnableMonitorActivity(bool isvalid, int timeout)
{
    _loop->RunInLoop(std::bind(&Connection::EnableTimerInLoop, this, isvalid, timeout));
}

// 更新回调
void Connection::UpgradeInLoop(ConnectedCallBack con_cb,
                               MessageCallBack msg_cb,
                               ClosedCallBack  close_cb,
                               AnyEventCallBack event_cb)
{
    _conn_cb   = _conn_cb;
    _msg_cb    = msg_cb;
    _closed_cb = _closed_cb;
    _any_cb    = event_cb;
}

void Connection::SetConnectedCallBack(const ConnectedCallBack& call_back) 
{
    _conn_cb = call_back;
}

void Connection::SetMessageCallBack(const MessageCallBack& call_back)
{
    _msg_cb = call_back;
}

void Connection::SetClosedCallBack(const ClosedCallBack& call_back)
{
    _closed_cb = call_back;
}

void Connection::SetAnyEventCallBack(const AnyEventCallBack& call_back)
{
    _any_cb = call_back;
}

void Connection::SetSrvClosedCallBack(const ClosedCallBack& call_back)
{
    _svr_closed_cb = call_back;
}
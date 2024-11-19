#include "../include/TcpServer.h"

TcpServer::TcpServer(int port, int thread_num)
    : _port(port), _conn_id(0), _istmract(false), _timeout(0)
    , _base_loop(std::make_shared<EventLoop>())
    , _acceptor(std::make_shared<Acceptor>(_port, _base_loop))
    , _pool(std::make_shared<LoopThreadPool>(thread_num, _base_loop))
{
    // 设置回调
    _acceptor->SetAcceptCallBack(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
    // 开启监听
    _acceptor->Listen();
    // 创建从属线程
    _pool->InitPool();
}

TcpServer::~TcpServer() = default;

void TcpServer::EnableMonitorActivity(bool isvalid, int sec)
{
    _istmract = isvalid;
    _timeout  = sec;
}

void TcpServer::NewConnection(int newfd)
{
    // 创建一个新的连接
    ConnPtr new_conn = std::make_shared<Connection>(_conn_id, newfd, _pool->NextLoop());

    // 设置回调函数
    new_conn->SetConnectedCallBack(_conn_cb);
    new_conn->SetMessageCallBack(_msg_cb);
    new_conn->SetClosedCallBack(_closed_cb);
    new_conn->SetSrvClosedCallBack(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

    // 是否启用活跃度监控
    if (_istmract) { new_conn->EnableMonitorActivity(true, _timeout); }
    else { new_conn->EnableMonitorActivity(false, _timeout); }
    
    // 开启监听
    new_conn->Established();
    
    _conns.insert(std::make_pair(_conn_id, new_conn));
    _conn_id++;
}

void TcpServer::RemoveConnInLoop(ConnPtr conn)
{
    int id = conn->GetId();
    if (_conns.find(id) != _conns.end()) { _conns.erase(id); }
}

void TcpServer::RemoveConnection(ConnPtr conn)
{
    _base_loop->RunInLoop(std::bind(&TcpServer::RemoveConnInLoop, this, conn));
}

void TcpServer::Start()
{
    while (1)
    {
        _base_loop->Start();
    }
}

void TcpServer::SetConnectedCallBack(const ConnectedCallBack& call_back) { _conn_cb = call_back; }

void TcpServer::SetMessageCallBack(const MessageCallBack& call_back) { _msg_cb = call_back; }

void TcpServer::SetClosedCallBack(const ClosedCallBack& call_back) { _closed_cb = call_back; }

void TcpServer::SetAnyEventCallBack(const AnyEventCallBack& call_back) { _any_cb = call_back; }

void TcpServer::SetSrvClosedCallBack(const ClosedCallBack& call_back) { _svr_closed_cb = call_back; }
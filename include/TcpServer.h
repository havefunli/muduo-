#pragma once
#include "LoopThread.h"
#include "Acceptor.h"
#include "Connection.h"

using MapConnPtr = std::unordered_map<int, ConnPtr>;

class TcpServer
{
private:
    // 创建一个新的连接
    void NewConnection(int);
    
    void RemoveConnInLoop(ConnPtr);

    // 移除管理的连接
    void RemoveConnection(ConnPtr);

public:
    /*需要端口号和线程数量*/
    TcpServer(int, int);

    ~TcpServer();

    // 用户自定义在发生特定事件后的回调函数
    void SetConnectedCallBack(const ConnectedCallBack&); // 连接成功回调
    void SetMessageCallBack(const MessageCallBack&);     // 接受数据后的回调
    void SetClosedCallBack(const ClosedCallBack&);       // 连接关闭后的回调
    void SetAnyEventCallBack(const AnyEventCallBack&);   // 任何事件的回调
    void SetSrvClosedCallBack(const ClosedCallBack&);    // 服务器的关闭回调

    // 是否启用非活跃挂断功能
    void EnableMonitorActivity(bool, int sec = DefaultTimeout);

    // 启动函数
    void Start();

private:
    uint16_t       _port;      // 监听的端口号
    int            _conn_id;   // 分配自增的连接ID
    bool           _istmract;  // 是否激活计时器
    int            _timeout;   // 延迟时间
    EveLoopPtr     _base_loop; // 主线程的loop，监视监听套接字
    AcceptorPtr    _acceptor;  // 管理监听套接字对象
    MapConnPtr     _conns;     // 记录每一个ID对应的Conn
    PoolPtr        _pool;      // 从属EventLoop线程池
    
    /*接受设置的回调函数，传递给 Connection*/
    ConnectedCallBack _conn_cb;
    MessageCallBack   _msg_cb;
    ClosedCallBack    _closed_cb;
    AnyEventCallBack  _any_cb;
    ClosedCallBack    _svr_closed_cb;  
};
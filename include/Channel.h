/*
* CopyRight Limuyou
* Date 24_11_5
* brief 对连接对应事件的处理函数
*/

#pragma once
#include <memory>
#include <sys/epoll.h>
#include <spdlog/spdlog.h>
#include <functional>

class EventLoop; // 前向声明
class Channel; // 前向声明
using EventFunc  = std::function<void()>;
using ChannelPtr = std::shared_ptr<Channel>;
using EveLoopPtr = std::shared_ptr<EventLoop>;

class Channel
{
public:
    
    // 无参数的构造, 但是后续一定要设置 FD ！！！
    Channel();

    // 构造函数，传入文件描述符
    explicit Channel(int fd);

    // 设置 EventLoop 管理器
    void SetEventLoop(EventLoop* loop);

    // 设置触发的事件
    void SetRevents(uint32_t revents);

    // 获取文件描述符
    int GetFd();

    // 设置文件描述符
    void SetFd(int);

    // 获取关注的事件
    uint32_t GetEvents();

    // 获取触发的事件
    uint32_t GetREvents();

    // 注册事件回调函数
    void RigisterEventsFunc(EventFunc readFunc, EventFunc writeFunc, EventFunc exceptFunc);

    // 设置关闭事件的回调函数
    void SetCloseFunc(EventFunc closeFunc);

    // 设置任意事件触发后的回调函数
    void SetEventCallBack(EventFunc eventCB);

    // 启用或禁用可读事件
    void EnableReadable(bool button);

    // 启用或禁用可写事件
    void EnableWriteable(bool button);

    // 处理触发的事件
    void HandleEvent();

    // 移除监控
    void ReMove();

    // 修改关注的事件
    void Update();

private:
    int        _fd;          // 关心的连接
    uint32_t   _events;      // 关注的事件
    uint32_t   _revents;     // 实际触发的事件
    EventLoop* _loop;        // EventLoop管理器

    EventFunc _read_cb;      // 读事件回调函数
    EventFunc _write_cb;     // 写事件回调函数
    EventFunc _except_cb;    // 异常事件回调函数
    EventFunc _close_cb;     // 断开连接回调函数
    EventFunc _event_cb;     // 任何事件处理后的回调函数
};

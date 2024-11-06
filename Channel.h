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

class Epoll; // 前向声明
using EventFunc = std::function<void()>;
using EpollPtr  = std::shared_ptr<Epoll>;

class Channel
{
public:
    // 构造函数，传入文件描述符
    explicit Channel(int fd);

    // 设置 epoll 管理器
    void SetEpoll(EpollPtr epoll);

    // 设置触发的事件
    void SetRevents(uint32_t revents);

    // 获取文件描述符
    int GetFd();

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

    // 从 epoll 中移除该文件描述符
    void EpollReMove();

    // 修改关注的事件
    void EpollModEvents();

private:
    int      _fd;            // 关心的连接
    uint32_t _events;        // 关注的事件
    uint32_t _revents;       // 实际触发的事件
    EpollPtr _epoll;         // epoll 管理器

    EventFunc _read_cb;      // 读事件回调函数
    EventFunc _write_cb;     // 写事件回调函数
    EventFunc _except_cb;    // 异常事件回调函数
    EventFunc _close_cb;     // 断开连接回调函数
    EventFunc _event_cb;     // 任何事件处理后的回调函数
};

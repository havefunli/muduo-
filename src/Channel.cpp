#include "../include/Channel.h"
#include "../include/EventLoop.h"

Channel::Channel()
    : _events(0), _revents(0), _loop(nullptr)
{}

Channel::Channel(int fd)
    : _fd(fd), _events(0), _revents(0), _loop(nullptr)
{}

void Channel::SetEventLoop(EventLoop* loop)
{
    _loop = loop;
}

void Channel::SetRevents(uint32_t revents)
{
    _revents = revents;
}

int Channel::GetFd()
{
    return _fd;
}

void Channel::SetFd(int fd)
{
    _fd = fd;
}

uint32_t Channel::GetEvents()
{
    return _events;
}

uint32_t Channel::GetREvents()
{
    return _revents;
}

void Channel::RigisterEventsFunc(EventFunc readFunc, EventFunc writeFunc, EventFunc exceptFunc)
{
    _read_cb = readFunc;
    _write_cb = writeFunc;
    _except_cb = exceptFunc;
}

void Channel::SetCloseFunc(EventFunc closeFunc)
{
    _close_cb = closeFunc;
}

void Channel::SetEventCallBack(EventFunc eventCB)
{
    _event_cb = eventCB;
}

void Channel::EnableReadable(bool button)
{
    if (button)
    {
        _events |= EPOLLIN;
    }
    else
    {
        _events &= ~EPOLLIN;
    }
    Update();
}

void Channel::EnableWriteable(bool button)
{
    if (button)
    {
        _events |= EPOLLOUT;
    }
    else
    {
        _events &= ~EPOLLOUT;
    }
    Update();
}

void Channel::HandleEvent()
{
    // 读事件
    if ((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI))
    {
        if (_read_cb) _read_cb();
    }
    
    // 写事件
    if (_revents & EPOLLOUT)
    {
        if (_write_cb) _write_cb();
    }
    // 异常事件
    else if (_revents & EPOLLERR)
    {
        if (_except_cb) _except_cb();
    }
    // 挂断事件
    else if (_revents & EPOLLHUP)
    {
        if (_close_cb) _close_cb();
    }

    // 任何事件处理之后的回调
    if (_event_cb) _event_cb();
}

void Channel::ReMove()
{
    if (_loop)
    {
        _loop->ReMoveEvent(this);
    }
}

void Channel::Update()
{
    if (_loop)
    {
        _loop->UpdateEvent(this);
    }
}

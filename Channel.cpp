#include "Channel.h"
#include "Epoll.h"

Channel::Channel(int fd)
    : _fd(fd), _events(0), _revents(0), _epoll(nullptr)
{}

void Channel::SetEpoll(EpollPtr epoll)
{
    _epoll = epoll;
}

void Channel::SetRevents(uint32_t revents)
{
    _revents = revents;
}

int Channel::GetFd()
{
    return _fd;
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
    EpollModEvents();
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
    EpollModEvents();
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
    if (_revents & EPOLLERR)
    {
        if (_except_cb) _except_cb();
    }

    // 挂断事件
    if (_revents & EPOLLHUP)
    {
        if (_close_cb) _close_cb();
    }

    // 任何事件处理之后的回调
    if (_event_cb) _event_cb();
}

void Channel::EpollReMove()
{
    if (_epoll)
    {
        _epoll->DelEvent(this);
    }
}

void Channel::EpollModEvents()
{
    if (_epoll)
    {
        if (_epoll->FindChannel(this->_fd).first)
        {
            _epoll->ModEvent(this);
        }
        else
        {
            _epoll->AddEvent(this);
        }
            
    }
}

#pragma once
#include "Epoll.h"
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <functional>
#include <sys/eventfd.h>

class EventLoop;
class TimerWheel;

using TaskFunc   = std::function<void()>;
using TaskVec    = std::vector<TaskFunc>;
using EveLoopPtr = std::shared_ptr<EventLoop>;
using WheelPtr   = std::shared_ptr<TimerWheel>;

class EventLoop
{
private:
    int CreateEventFd();

    bool IsInLoop();

public:
    EventLoop();
    
    // 执行任务池中的任务
    void RunAllTask();

    void RunInLoop(const TaskFunc& cb);

    void PushTask(const TaskFunc& cb);

    void ReadEventFd();

    void WakeUpEventFd();

    void Start();

    void UpdateEvent(Channel*);

    void ReMoveEvent(Channel*);

    void AddTimer(uint32_t id, uint32_t timeout, TaskFunc task);

    void RefreshTimer(uint32_t id);

    void EnableTimer(uint32_t id, bool isvalid);

private:
    int             _event_fd;
    TaskVec         _tasks;
    EpollPtr        _epoll;
    std::mutex      _mtx;
    ChannelPtr      _channel;
    std::thread::id _thread_id;
    WheelPtr        _timer_wheel;
};
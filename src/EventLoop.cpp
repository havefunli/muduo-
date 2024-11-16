#include "../include/EventLoop.h"
#include "../include/TimerWheel.h"

EventLoop::EventLoop()
    : _thread_id(std::this_thread::get_id())
    , _event_fd(CreateEventFd())
    , _epoll(std::make_shared<Epoll>())
    , _channel(std::make_shared<Channel>(_event_fd))
    , _timer_wheel(std::make_shared<TimerWheel>())
{
    _timer_wheel->SetEventLoop(this);
    _channel->SetEventLoop(this);
    _channel->RigisterEventsFunc(std::bind(&EventLoop::ReadEventFd, this), nullptr, nullptr);
    _channel->EnableReadable(true);
    
    spdlog::info("Successfully created EventLoop...");
}

int EventLoop::CreateEventFd()
{
    int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (efd < 0)
    {
        perror("eventfd");
        exit(-1);
    }

    return efd;
}

void EventLoop::ReadEventFd()
{
    int res = 0;
    int ret = read(_event_fd, &res, sizeof(res));
    if (ret < 0)
    {
        perror("read");
        exit(-1);
    }
}

bool EventLoop::IsInLoop()
{
    return _thread_id == std::this_thread::get_id();
}

void EventLoop::RunAllTask()
{
    std::vector<TaskFunc> functors;
    {
        std::unique_lock<std::mutex> lck(_mtx);
        functors.swap(_tasks);
    }

    for (auto& func : functors)
    {
        func();
    }

    return;
}

void EventLoop::WakeUpEventFd()
{
    int val = 1;
    int ret = write(_event_fd, &val, sizeof(val));
    if (ret < 0)
    {
        if (errno == EINTR || errno == EAGAIN) return;
        perror("write");
        exit(-1);
    }
}

void EventLoop::PushTask(const TaskFunc& cb)
{
    {
        std::unique_lock<std::mutex> lck(_mtx);
        _tasks.push_back(cb);
    }
    WakeUpEventFd();
}

void EventLoop::RunInLoop(const TaskFunc& cb)
{
    if (IsInLoop())
        return cb();
    return PushTask(cb);
}

void EventLoop::Start()
{
    // 1. 事件监控
    std::vector<Channel*> actives; 
    _epoll->Wait(actives);
    
    // 2. 事件处理
    for (auto& channel : actives)
    {
        channel->HandleEvent();
    }

    // 3. 执行任务
    RunAllTask();
}

void EventLoop::UpdateEvent(Channel* channel)
{
    if (_epoll->FindChannel(channel->GetFd()).first)
    {
        _epoll->ModEvent(channel);
    }
    else
    {
        _epoll->AddEvent(channel);
    }
}

void EventLoop::ReMoveEvent(Channel* channel)
{
    _epoll->DelEvent(channel);
    spdlog::debug("Epoll rm fd = {}", channel->GetFd());
}

void EventLoop::AddTimer(uint32_t id, uint32_t timeout, TaskFunc task)
{
    _timer_wheel->AddTimer(id, timeout, task);
}

void EventLoop::RefreshTimer(uint32_t id)
{
    _timer_wheel->RefreshTimer(id);
}

bool EventLoop::HasTimer(uint32_t id)
{
    return _timer_wheel->IsExists(id).first;
}

void EventLoop::EnableTimer(uint32_t id, bool isvalid)
{
    _timer_wheel->EnableTimer(id, isvalid);
}



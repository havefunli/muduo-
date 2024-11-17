#include "../include/TimerWheel.h"


TimerTask::TimerTask(uint32_t id, uint32_t timeout, TaskFunc func)
    : _isvalid(true), _task_id(id), _timeout(timeout), _task_cb(func)
{}

TimerTask::~TimerTask()
{
    _release_cb();
    if (_isvalid ) { _task_cb(); }
}

uint32_t TimerTask::TaskId()
{
    return _task_id;
}

uint32_t TimerTask::TimeDelay()
{
    return _timeout;
}

bool TimerTask::IsValid()
{
    return _isvalid;
}

TaskFunc TimerTask::Func()
{
    return _task_cb;
}

void TimerTask::SetRelease(TaskFunc Release)
{
    _release_cb = Release;
}

void TimerTask::SetValid(bool isvalid)
{
    _isvalid = isvalid;
}


uint32_t TimerWheel::GetNextLocation(uint32_t timeout)
{
    return (_tick + timeout) % _capacity;
}

FindRut TimerWheel::IsExists(uint32_t id)
{
    auto pos = _timers.find(id);
    if (pos == _timers.end()) return {false, WeakTask()};
    return {true, pos->second};
}

int TimerWheel::CreateTimerFd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd < 0)
    {
        perror("timer_create");
        return -1;
    }

    struct itimerspec itime;
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0;
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0;
    ::timerfd_settime(timerfd, 0, &itime, nullptr);

    return timerfd;
}

void TimerWheel::ReadTimerFd()
{
    uint64_t ret = 0;
    int n = read(_timer_fd, &ret, sizeof(ret));
    if (n < 0)
    {
        perror("read");
        return;
    }
    return;
}

bool TimerWheel::AddTimerInLoop(uint32_t id, uint32_t timeout, TaskFunc task)
{
    if (IsExists(id).first)
    {
        spdlog::error("Add error, the id = {} is exists...", id);
        return false;
    } 

    // 初始化一个 Timer 对象，绑定一个回调函数
    PtrTask sp = std::make_shared<TimerTask>(id, timeout, task);
    sp->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));
    _wheel[GetNextLocation(timeout)].push_back(sp);

    // 将该对象保存起来
    WeakTask wp = sp;
    _timers.insert(make_pair(id, wp));
    spdlog::info("Successful add id = {} to TimerWheel...", id);

    return true;
}

// 当连接活跃时，重新计时
bool TimerWheel::RefreshTimerInLoop(uint32_t id)
{
    FindRut pair = IsExists(id);
    if (!pair.first)
    {
        spdlog::error("Refresh error, the id = {} is not exists...", id);
        return false;
    }

    PtrTask sp = pair.second.lock();
    // TimerAdd(sp->TaskId(), sp->TimeDelay(), sp->Func());  // 在这里踩了坑，不可以代码复用
    // 若定时器无效则直接退出                                 // 我们需要智能指针指向同一个对象，但是调用 add 会在生成一个新的
    if (!sp->IsValid()) { return true; }
    int timeout = sp->TimeDelay();                           
    int pos = GetNextLocation(timeout);
    _wheel[pos].push_back(sp);

    return true;
}

bool TimerWheel::EnableTimerInLoop(uint32_t id, bool isvalid)
{
    FindRut pair = IsExists(id);
    if (!pair.first)
    {
        spdlog::error("Cancle error, the id = {} is not exists...", id);
        return false;
    }

    PtrTask sp = pair.second.lock();
    sp->SetValid(isvalid);
    return true;
}

TimerWheel::TimerWheel(uint32_t TimeLimit)
    : _capacity(TimeLimit), _wheel(_capacity), _tick(0)
    , _timer_fd(CreateTimerFd()), _channel(std::make_shared<Channel>(_timer_fd))
{
    // 开启监控，以及回调方法
    _channel->RigisterEventsFunc(std::bind(&TimerWheel::OnTime, this), nullptr, nullptr);
    spdlog::info("Successful init timerwheel...");
}

// 需要控制一下析构顺序，保证 _timer 后析构
// 因为再调用 _wheel 的析构时，会触发 TimerRemove(uint32_t id) 函数
// 如果前者已经析构，会报错
TimerWheel::~TimerWheel()
{
    _wheel.clear();
    _timers.clear();
}

void TimerWheel::OnTime()
{
    LoopOnce();
    ReadTimerFd();
}

void TimerWheel::SetEventLoop(EventLoop* loop)
{
    _loop = loop;
    // 开启对 timerfd 的监控, 在这里踩了雷
    _channel->SetEventLoop(loop);
    _channel->EnableReadable(true);
}

void TimerWheel::RemoveTimer(uint32_t id)
{
    if (!IsExists(id).first) return;

    _timers.erase(id);
    spdlog::info("Successful remove id = {}...", id);
}

void TimerWheel::AddTimer(uint32_t id, uint32_t timeout, TaskFunc task)
{
    _loop->RunInLoop(std::bind(&TimerWheel::AddTimerInLoop, this, id, timeout, task));
}

void TimerWheel::RefreshTimer(uint32_t id)
{
    _loop->RunInLoop(std::bind(&TimerWheel::RefreshTimerInLoop, this, id));
}

void TimerWheel::EnableTimer(uint32_t id, bool isvalid)
{
    _loop->RunInLoop(std::bind(&TimerWheel::EnableTimerInLoop, this, id, isvalid));
}

void TimerWheel::LoopOnce()
{
    _tick = (_tick + 1) % _capacity;
    _wheel[_tick].clear();
}

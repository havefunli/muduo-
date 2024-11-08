/*
* CopyRight Limuyou
* Date 24_10_28
* brief 实现了一个时间轮，用于关闭不活跃的连接
*/

#pragma once
#include "EventLoop.h"

#include <signal.h>
#include <time.h>
#include <sys/timerfd.h>

#include <memory>
#include <functional>

#include <utility>
#include <vector>
#include <list>
#include <unordered_map>

class TimerTask;
class TimerWheel;

const int DefaultTimeLimit = 60;

using TaskFunc = std::function<void()>;
using WheelPtr = std::shared_ptr<TimerWheel>;
using PtrTask  = std::shared_ptr<TimerTask>;
using WeakTask = std::weak_ptr<TimerTask>;
using FindRut  = std::pair<bool, WeakTask>;

// 时间轮中的每一个对象
// 需要指定超时时间，回调函数
class TimerTask
{
public:
    TimerTask(uint32_t id, uint32_t timeout, TaskFunc func);

    ~TimerTask();

    uint32_t TaskId();

    uint32_t TimeDelay();

    TaskFunc Func();

    void SetRelease(TaskFunc Release);

    void SetValid(bool isvalid);

private:
    bool     _isvalid;    // 该任务是否生效
    uint32_t _task_id;    // 任务 id
    uint32_t _timeout;    // 任务超时时间
    TaskFunc _task_cb;    // 定时任务
    TaskFunc _release_cb; // 清除回调
};

// 时间轮的实现
class TimerWheel
{
private:
    uint32_t GetNextLocation(uint32_t timeout);

    FindRut IsExists(uint32_t id);

    int CreateTimerFd();

    void ReadTimerFd();

    bool AddTimerInLoop(uint32_t id, uint32_t timeout, TaskFunc task);

    // 当连接活跃时，重新计时
    bool RefreshTimerInLoop(uint32_t id);

    bool EnableTimerInLoop(uint32_t id, bool isvalid);

public:
    TimerWheel(uint32_t TimeLimit = DefaultTimeLimit);

    // 需要控制一下析构顺序，保证 _timer 后析构
    // 因为再调用 _wheel 的析构时，会触发 TimerRemove(uint32_t id) 函数
    // 如果前者已经析构，会报错
    ~TimerWheel();

    void OnTime();

    void SetEventLoop(EveLoopPtr loop);

    void RemoveTimer(uint32_t id);

    void AddTimer(uint32_t id, uint32_t timeout, TaskFunc task);
    
    void RefreshTimer(uint32_t id);
    
    void EnableTimer(uint32_t id, bool isvalid);

    void LoopOnce();

private:
    int        _timer_fd; // 使用 EventLoop 管理定时
    EveLoopPtr _loop;
    ChannelPtr _channel;

    uint32_t                               _capacity; // 轮盘最大容量
    uint32_t                               _tick;     // 秒针
    std::vector<std::list<PtrTask>>        _wheel;    // 轮盘
    std::unordered_map<uint32_t, WeakTask> _timers;   // 保存所有轮盘中的任务
};

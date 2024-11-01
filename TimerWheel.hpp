#pragma once

#include <unistd.h>
#include <iostream>
#include <memory>
#include <functional>
#include <utility>
#include <vector>
#include <list>
#include <unordered_map>

class TimerTask;

using TaskFunc = std::function<void()>;
using PtrTask  = std::shared_ptr<TimerTask>;
using WeakTask = std::weak_ptr<TimerTask>;
using FindRut  = std::pair<bool, WeakTask>;
// 时间轮中的每一个对象
// 需要指定超时时间，回调函数
class TimerTask
{
public:
    TimerTask(uint32_t id, uint32_t timeout, TaskFunc func)
        : _isvalid(true), _task_id(id), _timeout(timeout), _task_cb(func)
    {}

    ~TimerTask()
    {
        if (!_isvalid) return;
        _task_cb();
        _release_cb();
    }

    uint32_t TaskId()
    {
        return _task_id;
    }

    uint32_t TimeDelay()
    {
        return _timeout;
    }

    TaskFunc Func()
    {
        return _task_cb;
    }

    void SetRelease(TaskFunc Release)
    {
        _release_cb = Release;
    }

    void SetValid(bool isvalid)
    {
        _isvalid = isvalid;
    }

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
    uint32_t GetNextLocation(uint32_t timeout)
    {
        return (_tick + timeout) % _capacity;
    }

    FindRut IsExists(uint32_t id)
    {
        auto pos = _timers.find(id);
        if (pos == _timers.end()) return {false, WeakTask()};
        return {true, pos->second};
    }

public:
    TimerWheel(uint32_t TimeLimit)
        : _capacity(TimeLimit), _wheel(_capacity), _tick(0)
    {
        std::cout << "Successful init timerwheel..." << std::endl;
    }


    // 需要控制一下析构顺序，保证 _timer 后析构
    // 因为再调用 _wheel 的析构时，会触发 TimerRemove(uint32_t id) 函数
    // 如果前者已经析构，会报错
    ~TimerWheel()
    {
        _wheel.clear();
        _timers.clear();
    }

    void TimerRemove(uint32_t id)
    {
        if (!IsExists(id).first) return;

        _timers.erase(id);
        std::cout << "Successful remove id = " << id << "..." << std::endl;
    }

    bool TimerAdd(uint32_t id, uint32_t timeout, TaskFunc task)
    {
        if (IsExists(id).first)
        {
            std::cerr << "Add error, the id = " << id << " is exists..." << std::endl;
            return false;
        } 

        // 初始化一个 Timer 对象，绑定一个回调函数
        PtrTask sp = std::make_shared<TimerTask>(id, timeout, task);
        sp->SetRelease(std::bind(&TimerWheel::TimerRemove, this, id));
        _wheel[GetNextLocation(timeout)].push_back(sp);

        // 将该对象保存起来
        WeakTask wp = sp;
        _timers.insert(make_pair(id, wp));
        std::cout << "Successful add id = " << id << " to TimerWheel..." << std::endl;

        return true;
    }

    // 当连接活跃时，重新计时
    bool TimerRefresh(uint32_t id)
    {
        FindRut pair = IsExists(id);
        if (!pair.first)
        {
            std::cerr << "Refresh error, the id = " << id << " is not exists..." << std::endl;
            return false;
        }

        PtrTask sp = pair.second.lock();
        // TimerAdd(sp->TaskId(), sp->TimeDelay(), sp->Func());  // 在这里踩了坑，不可以代码复用
        int timeout = sp->TimeDelay();                           // 我们需要智能指针指向同一个对象，但是调用 add 会在生成一个新的
        int pos = GetNextLocation(timeout);
        _wheel[pos].push_back(sp);

        return true;
    }

    bool EnableTimer(uint32_t id, bool isvalid)
    {
        FindRut pair = IsExists(id);
        if (!pair.first)
        {
            std::cerr << "Cancle error, the id = " << id << " is not exists..." << std::endl;
            return false;
        }

        PtrTask sp = pair.second.lock();
        sp->SetValid(isvalid);
    }

    void Loop()
    {
        _tick = (_tick + 1) % _capacity;
        _wheel[_tick].clear();
        sleep(1);
    }

private:
    uint32_t                               _capacity; // 轮盘最大容量
    uint32_t                               _tick;     // 秒针
    std::vector<std::list<PtrTask>>        _wheel;    // 轮盘
    std::unordered_map<uint32_t, WeakTask> _timers;   // 保存所有轮盘中的任务
};
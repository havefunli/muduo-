#pragma once

#include <unistd.h>
#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <list>
#include <unordered_map>

class TimerTask;

using TaskFunc = std::function<void()>;
using PtrTask  = std::shared_ptr<TimerTask>;
using WeakTask = std::weak_ptr<TimerTask>;

class TimerTask
{
public:
    TimerTask(uint32_t id, uint32_t timeout, TaskFunc func)
        : _task_id(id), _timeout(timeout), _task_cb(func)
    {
    }

    ~TimerTask()
    {
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

private:
    uint32_t _task_id;    // 任务 id
    uint32_t _timeout;    // 任务超时时间
    TaskFunc _task_cb;    // 定时任务
    TaskFunc _release_cb; // 清除回调
};

class TimerWheel
{
private:
    uint32_t GetLoc(uint32_t timeout)
    {
        return (_tick + timeout) % _capacity;
    }

public:
    TimerWheel(uint32_t TimeLimit)
        : _capacity(TimeLimit), _wheel(_capacity), _tick(0)
    {
        std::cout << "Successful init timerwheel..." << std::endl;
    }

    bool TimerRemove(uint32_t id)
    {
        auto pos = _timers.find(id);
        if (pos == _timers.end())
        {
            std::cerr << "The id = %d is exists..." << id << std::endl;
            return false;
        }

        _timers.erase(id);

        std::cout << "Successful remove fd = %d ..." << id << std::endl;

        return true;
    }

    bool TimerAdd(uint32_t id, uint32_t timeout, TaskFunc task)
    {
        if (_timers.find(id) != _timers.end())
        {
            std::cerr << "The id = %d is exists..." << id << std::endl;

            return false;
        }

        PtrTask sp = std::make_shared<TimerTask>(id, timeout, task);

        sp->SetRelease(std::bind(&TimerWheel::TimerRemove, this, id));

        _wheel[GetLoc(timeout)].push_back(sp);

        WeakTask wp = sp;

        _timers.insert(make_pair(id, wp));

        std::cout << "Successful add id = %d to TimerWheel..." << id << std::endl;

        return true;
    }

    bool TimerRefresh(uint32_t id)

    {

        auto pos = _timers.find(id);
        if (pos == _timers.end())
        {
            std::cerr << "The id = %d is not exists..." << id << std::endl;

            return false;
        }

        PtrTask sp = pos->second.lock();
        TimerAdd(sp->TaskId(), sp->TimeDelay(), sp->Func());
    }

    void Loop()
    {
        _tick = (_tick + 1) % _capacity;
        _wheel[_tick].clear();
        sleep(1);
    }

private:
    uint32_t _capacity; // 轮盘最大容量
    uint32_t _tick; // 秒针
    std::vector<std::list<PtrTask>> _wheel; // 轮盘
    std::unordered_map<uint32_t, WeakTask> _timers; // 保存所有轮盘中的任务
};
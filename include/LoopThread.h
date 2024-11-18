
#pragma once
#include "EventLoop.h"
#include <thread>
#include <mutex>
#include <condition_variable>

class LoopThread
{
private:    
    void ThreadEntrance();

public:
    LoopThread();

    ~LoopThread();

    // 上锁操作，只有初始化了 loop 才能获取
    EveLoopPtr GetLoop();

private:
    /*每一个线程负责一个loop*/
    EveLoopPtr               _loop;   // 初始化一个 loop
    std::thread              _thread; // 创建一个线程
    std::mutex               _mtx;    // 对获取 loop 上锁操作
    std::condition_variable  _cond;   // 和上锁操作配合
};
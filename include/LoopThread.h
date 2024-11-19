#pragma once
#include "EventLoop.h"
#include <thread>
#include <mutex>
#include <condition_variable>

class LoopThread;

using LoopThreadPtr = std::shared_ptr<LoopThread>;
using LoopThreads   = std::vector<LoopThreadPtr>;

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


/*将 LoopThread 封装为线程池*/
class LoopThreadPool
{
private:
    void CreateThread();

    void IndexMove();

public:
    LoopThreadPool(int thread_num, EveLoopPtr base_loop);

    ~LoopThreadPool();

    void InitPool();

    EveLoopPtr NextLoop();

private:
    int         _thread_num;    // 线程数量
    int         _loop_index;    // 下一个 loop 下标
    EveLoopPtr  _base_loop;     // 连接分配的主 loop
    LoopThreads _threads;       
};
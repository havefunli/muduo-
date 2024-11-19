#include "../include/LoopThread.h"

LoopThread::LoopThread()
    : _loop(nullptr), _thread(std::thread(&LoopThread::ThreadEntrance, this))
{}

LoopThread::~LoopThread() = default;

void LoopThread::ThreadEntrance()
{
    {
        std::unique_lock lck(_mtx);
        _loop = std::make_shared<EventLoop>();
        _cond.notify_all();
    }
    while (1) { _loop->Start(); }
}

EveLoopPtr LoopThread::GetLoop()
{
    std::unique_lock lck(_mtx);
    _cond.wait(lck, [&](){ return _loop != nullptr; });
    return _loop;
}


/*--------------------------------------------------------*/


LoopThreadPool::LoopThreadPool(int thread_num, EveLoopPtr base_loop)
    : _thread_num(thread_num), _loop_index(0), _base_loop(base_loop)
{}

LoopThreadPool::~LoopThreadPool() = default;

void LoopThreadPool::CreateThread()
{
    // 如果为 0 ，直接退出
    if (_thread_num)
    {
        _threads.reserve(_thread_num);
        for (int i = 0; i < _thread_num; i++)
        {
            _threads[i] = std::make_shared<LoopThread>();
        }
    }
    return;
}

void LoopThreadPool::IndexMove()
{
    _loop_index = (_loop_index + 1) % _thread_num;
}

void LoopThreadPool::InitPool() { CreateThread(); }

EveLoopPtr LoopThreadPool::NextLoop()
{
    /*如果设置 0 个线程，则使用主loop*/
    if (_thread_num == 0) return _base_loop;

    EveLoopPtr loop = _threads[_loop_index]->GetLoop();
    IndexMove();
    return loop;
}


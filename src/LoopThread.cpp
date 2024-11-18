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

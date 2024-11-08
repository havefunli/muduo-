#include "../include/Epoll.h"

// 构造函数：初始化 epoll
Epoll::Epoll()
{
    _epfd = epoll_create(DefaultSize);
    if (_epfd < 0)
    {
        perror("epoll_create");
        exit(1);
    }
    spdlog::info("Successfully created epoll...");
}

// 析构函数
Epoll::~Epoll()
{
    close(_epfd);
}

// 创建 epoll_event 并设置为 ET 模式
struct epoll_event Epoll::CreateEpollEvent(int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLET | events;  // 使用 ET 模式
    return ev;
}

// 辅助方法：修改事件
bool Epoll::ModEventHelper(Channel* channel, int op)
{
    struct epoll_event ev = CreateEpollEvent(channel->GetFd(), channel->GetEvents());
    int n = epoll_ctl(_epfd, op, channel->GetFd(), &ev);
    if (n < 0)
    {
        perror("epoll_ctl");
        return false;
    }
    return true;
}

// 查找 Channel 是否存在
SearchResult Epoll::FindChannel(int fd)
{
    auto iter = _channels.find(fd);
    return iter == _channels.end() ? std::make_pair(false, nullptr) : std::make_pair(true, iter->second);
}

// 增加事件
bool Epoll::AddEvent(Channel* channel)
{
    if (ModEventHelper(channel, EPOLL_CTL_ADD))
    {
        _channels.insert(std::make_pair(channel->GetFd(), channel));
        spdlog::debug("Epoll add fd = {}", channel->GetFd());
        return true;
    }
    return false;
}

// 修改事件
bool Epoll::ModEvent(Channel* channel)
{
    spdlog::debug("Epoll mod fd = {}", channel->GetFd());
    return ModEventHelper(channel, EPOLL_CTL_MOD);
}

// 删除事件
bool Epoll::DelEvent(Channel* channel)
{
    if (ModEventHelper(channel, EPOLL_CTL_DEL))
    {
        _channels.erase(channel->GetFd());
        spdlog::debug("Epoll rm fd = {}", channel->GetFd());
        return true;
    }
    return false;
}

// 等待事件发生
int Epoll::Wait(std::vector<Channel*>& active_channel)
{
    int n = epoll_wait(_epfd, _rev, DefaultSize, -1);
    if (n < 0)
    {
        perror("epoll_wait");
    }
    
    for (int i = 0; i < n; i++)
    {
        int fd = _rev[i].data.fd;
        auto result = FindChannel(fd);
        if (!result.first) continue;  // 如果没有找到对应的 Channel，跳过
        result.second->SetRevents(_rev[i].events);  // 设置事件
        active_channel.push_back(result.second);    // 返回
    }

    return n;
}

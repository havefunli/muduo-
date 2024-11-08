/*
* CopyRight Limuyou
* Date 24_11_5
* brief 对连接的事件管理
*/

#pragma once
#include "Channel.h"
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <vector>
#include <unordered_map>

class Epoll;

const int DefaultSize = 1024;
using EpollPtr     = std::shared_ptr<Epoll>;
using ChannelMap   = std::unordered_map<int, Channel*>;
using SearchResult = std::pair<bool, Channel*>;

class Epoll
{
private:
    // 创建 epoll_event 并设置为 ET 模式
    struct epoll_event CreateEpollEvent(int fd, uint32_t events);

    // 辅助方法：修改事件
    bool ModEventHelper(Channel* channel, int op);

public:
    // 构造函数
    Epoll();

    // 析构函数
    ~Epoll();

    // 查找 Channel
    SearchResult FindChannel(int fd);

    // 增加事件
    bool AddEvent(Channel* channel);

    // 修改事件
    bool ModEvent(Channel* channel);

    // 删除事件
    bool DelEvent(Channel* channel);

    // 等待事件
    int Wait(std::vector<Channel*>&);

private:
    int         _epfd;               // epoll 文件描述符
    ChannelMap  _channels;           // 管理 Channel 的映射
    epoll_event _rev[DefaultSize];   // 事件接受缓冲区
};

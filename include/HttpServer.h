#pragma once
#include <regex>
#include "../include/TcpServer.h"
#include "../src/Http.hpp"

using Handler   = std::function<void(const HttpRequest&, HttpResponse&)>;
using MethodMap = std::unordered_map<std::string, Handler>;

class HttpServer
{
private:
    // 是否是文件请求
    bool IsFileHandle(HttpRequest&);

    // 静态资源请求
    void FileHandle(HttpRequest&, HttpResponse&);

    // 功能性请求处理
    void DispatchHandle(HttpRequest&, HttpResponse&, MethodMap);

    // 路由请求
    void Route(HttpRequest&, HttpResponse&);

    // 错误返回
    void ErrorHandle(HttpRequest&, HttpResponse&);

    // 缓冲区数据解析 + 处理
    void OnMessage(ConnPtr, BufferPtr); 

    // 设置上下文
    void OnConncted(ConnPtr);

    // 组织数据格式发送
    void SendResponse(ConnPtr, HttpRequest&, HttpResponse&);


public:
    HttpServer(int, int, std::string base_dir = "");
    
    ~HttpServer();

    void SetBaseDir(const std::string&);

    // 设置 GET 对应的函数
    void Get(const std::string&, Handler&);

    // 设置 Post 对应的函数
    void Post(const std::string&, Handler&);

    // 设置 Put 对应的函数
    void Put(const std::string&, Handler&);

    // 设置 Del 对应的函数
    void Del(const std::string&, Handler&);

    // 开始运行
    void Start();

private:
    MethodMap   _get_route;  // GET 对应的方法
    MethodMap   _post_route; // POST 对应的方法
    MethodMap   _put_route;  // PUT 对应的方法
    MethodMap   _del_route;  // DELETE 对应的方法
    std::string _base_dir;   // 静态资源目录
    TcpServer   _server;     // 负责管理连接
};
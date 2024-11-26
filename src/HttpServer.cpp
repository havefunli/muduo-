#include "../include/HttpServer.h"

void HttpServer::DispatchHandle(HttpRequest& req, HttpResponse& rsp, MethodMap handlers)
{
    // for (auto& handler : handlers)
    // {
    //     const std::string& method_name = handler.first;
    //     Handler func = handler.second;
    //     // 判断请求的方法是否存在
    //     bool ret = (method_name == req.GetSrc());
    //     // 判断使用哪个函数逻辑
    //     if (!ret) { continue; }
    //     return func(req, rsp);
    // }

    // 没有合适的方法
    if (handlers.find(req.GetSrc()) == handlers.end()) { return rsp.SetStatus(404); }
    // 执行
    else { return handlers[req.GetSrc()](req, rsp); }  
}

bool HttpServer::IsFileHandle(HttpRequest& req)
{
    // 1. 是否设置了静态资源路径
    if (_base_dir.empty()) { return false; }
    // 2. 请求方法是否是 GET
    if (req._method != "GET") { return false; }
    // 3. 请求的资源路径是否合法
    if (!Util::IsValidPath(req._path)) { return false; }
    // 4. 对 / 根目录的请求
    if (req._path.back() == '/') { req._path += "index.html"; }
    // 5. 加上资源根目录
    req._path = _base_dir + req._path;
    
    return true;
}

void HttpServer::Route(HttpRequest& req, HttpResponse& rsp)
{
    // 首先判断请求类型
    if (IsFileHandle(req)) { return FileHandle(req, rsp); }

    if (req._method == "GET") { return DispatchHandle(req, rsp, _get_route); }
    else if (req._method == "POST") { return DispatchHandle(req, rsp, _post_route); }
    else if (req._method == "PUT") { return DispatchHandle(req, rsp, _put_route); }
    else if (req._method == "DELETE") { return DispatchHandle(req, rsp, _del_route); }

    // 没有任何方式处理
    rsp.SetStatus(404);
}

void HttpServer::OnConncted(ConnPtr conn) 
{ 
    conn->SetContext(HttpContext()); 
}

void HttpServer::OnMessage(ConnPtr conn, BufferPtr buffer)
{
    // spdlog::debug("Now I am in the OnMsg...");
    // if (buffer == nullptr) 
    // {
    //     std::cout << "OnMessage" << std::endl;
    //     return; 
    // }

    
    // 避免多个请求只处理一个
    while (buffer->ReadableBytes())
    {
        // 1. 获取上下文
        HttpContext& context = conn->GetContext().Get<HttpContext>();
        // 2. 对现在 buffer 的数据解析到上下文, 并获取 request
        spdlog::debug("Start parsing the request data...");
        context.ParseHttpRequest(buffer);
        spdlog::debug("Parsing data done...");

        HttpRequest& req = context.GetRequest();
        HttpResponse rsp;
        rsp.SetStatus(context.RepStatus());
        // 请求有问题
        if (context.RepStatus() != 200) 
        {
            spdlog::info("HTTP format problem, disconnect!");
            // 需要清除缓冲区数据！
            // 不然当我们关闭连接时，会先判断有无数据处理，有的话还要处理，如此死循环！
            buffer->Clear();
            ErrorHandle(req, rsp);
            SendResponse(conn, req, rsp);
            conn->ShutDown();
            return;
        }
        // 数据不完整，不能处理
        if (context.RecvStatus() != HttpRecvStatus::OVER) { return; }
        // 3. 路由请求业务处理
        Route(req, rsp);
        // 4. 组织发送数据
        SendResponse(conn, req, rsp);
        // 5. 是否挂断链接
        if (req.Close()) 
        {
             spdlog::info("Ready to hang up...");
            conn->ShutDown(); 
             spdlog::info("Done...");
        }
        // 6. 重置上下文，为下一次数据做准备
        context.Clear();
    }
}

void HttpServer::ErrorHandle(HttpRequest& req, HttpResponse& rsp)
{

    std::string out;
    Util::ReadFile(_base_dir + "error.html", out);
    rsp.SetContent(out, "text/html");
}

void HttpServer::SendResponse(ConnPtr conn, HttpRequest& req, HttpResponse& rsp)
{
    std::string msg;
    // 获取响应数据
    rsp.InitMsg(msg);
    // 发送数据
    // spdlog::debug("msg = {}", msg);
    conn->Send(msg.c_str(), msg.size());
}

void HttpServer::FileHandle(HttpRequest& req, HttpResponse& rsp)
{
    // 读取指定文件内容
    std::string content;
    Util::ReadFile(req._path, content);
    // 获取文件类型
    std::string mime_type = Util::GetMimeType(req._path);
    // 放入响应结构体中
    rsp.SetContent(content, mime_type);
}


HttpServer::HttpServer(int port, int thread_num, std::string base_dir) 
    : _server(port, thread_num)
    , _base_dir(base_dir)
{
    // 设置回调函数
    _server.SetConnectedCallBack(std::bind(&HttpServer::OnConncted, this, std::placeholders::_1));
    _server.SetMessageCallBack(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    // 启动非活跃断开功能
    _server.EnableMonitorActivity(true);
}

HttpServer::~HttpServer()
{
    spdlog::debug("~HttpServer");
}

void HttpServer::SetBaseDir(const std::string& dir) { _base_dir = dir; }

void HttpServer::Get(const std::string& pattern, Handler& handle)
{
    _get_route.insert(std::make_pair(pattern, handle));
}

void HttpServer::Post(const std::string& pattern, Handler& handle)
{
    _get_route.insert(std::make_pair(pattern, handle));
}

void HttpServer::Put(const std::string& pattern, Handler& handle)
{
    _get_route.insert(std::make_pair(pattern, handle));
}

void HttpServer::Del(const std::string& pattern, Handler& handle)
{
    _get_route.insert(std::make_pair(pattern, handle));
}

void HttpServer::Start()
{
    _server.Start();
}
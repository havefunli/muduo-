#include "../include/HttpServer.h"

void HttpServer::DispatchHandle(HttpRequest& req, HttpResponse& rsp, MethodMap handlers)
{
    for (auto& handler : handlers)
    {
        const std::regex& re = handler.first;
        Handler func = handler.second;

        bool ret = std::regex_match(req._path, req._matches, re);
        if (!ret) { continue; }
        return func(req, rsp);
    }
    // 没有合适的方法
    return rsp.SetStatus(404);
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

void HttpServer::OnMessage(ConnPtr conn, BufferPtr buffer)
{
    // 避免多个请求只处理一个
    while (buffer->ReadableBytes())
    {
        // 1. 获取上下文
        HttpContext& context = conn->GetContext().Get<HttpContext>();
        // 2. 对现在 buffer 的数据解析到上下文, 并获取 request
        context.ParseHttpRequest(buffer);
        HttpRequest& req = context.GetRequest();
        
        HttpResponse rsp;
        rsp.SetStatus(context.RepStatus());
        
        // 请求有问题
        if (context.RepStatus() != 400) 
        {
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
        if (req.Close()) { conn->ShutDown(); }
        // 6. 重置上下文，为下一次数据做准备
        context.Clear();
    }
}

void HttpServer::OnConncted(ConnPtr conn) 
{ 
    conn->SetContext(HttpContext()); 
}

void HttpServer::ErrorHandle(HttpRequest& req, HttpResponse& rsp)
{

    std::string out;
    Util::ReadFile(TemplatePath + "error.html", out);
    rsp.SetContent(out, "text/html");
}

void HttpServer::SendResponse(ConnPtr conn, HttpRequest& req, HttpResponse& rsp)
{
    std::string msg;
    // 获取响应数据
    rsp.InitMsg(msg);
    // 发送数据
    conn->Send(msg.c_str(), msg.size());
}
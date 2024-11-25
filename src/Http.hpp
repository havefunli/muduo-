#pragma once
#include "../include/TcpServer.h"
#include "../src/Util.hpp"
#include <string>
#include <sstream>
#include <unordered_map>

class HttpRequest;

using HeaderMap = std::unordered_map<std::string, std::string>;
using ParamsMap = std::unordered_map<std::string, std::string>;

const std::string DefaultLineSep   = "\r\n";
const std::string DefaultHeaderSep = ": ";

class HttpRequest
{
public:
    std::string _method;  // 请求方法
    std::string _path;    // 资源路径
    std::string _version; // 协议版本
    std::string _body;    // 正文内容
    HeaderMap   _headers; // 请求头
    ParamsMap   _params;  // 请求参数

    bool HasHeader(const std::string &key)
    {
        auto iter = _headers.find(key);
        return iter != _headers.end();
    }

    void SetHeader(const std::string &key, const std::string &val)
    {
        if (_headers.find(key) == _headers.end())
        {
            _headers.insert(std::make_pair(key, val));
        }
    }

    std::string GetHeader(const std::string &key)
    {
        auto iter = _headers.find(key);
        if (iter != _headers.end())
        {
            return iter->second;
        }
        return "";
    }

    size_t GetContentLength()
    {
        auto iter = _headers.find("Content-Length");
        if (iter != _headers.end())
        {
            return std::stoi(GetHeader("Content-Length"));
        }
        return 0;
    }

    /*
    特定数据的获取:
    静态资源   -> 资源返回
    特定的功能 -> 功能返回
    */
    std::string GetSrc()
    {
        auto pos = _path.find_last_of('/');
        return _path.substr(pos + 1);
    }

    // 判断长短连接
    bool Close()
    {
        // 没有Connection字段，或者有Connection但是值是close，则都是短链接，否则就是长连接
        if (HasHeader("Connection") == true && GetHeader("Connection") == "keep-alive") 
        {
            return false;
        }
        return true;
    }

    // 清空数据
    void Clear()
    {
        _method = _path = _version = _body = "";
        _headers.clear();
        _params.clear();
    }
};

class HttpResponse
{
public:
    HttpResponse()
        : _redirect(false), _status(200)
    {}

    bool HashHeader(const std::string &key)
    {
        auto iter = _headers.find(key);
        return iter != _headers.end();
    }

    void SetHeader(const std::string &key, const std::string &val)
    {
        if (_headers.find(key) == _headers.end())
        {
            _headers.insert(std::make_pair(key, val));
        }
    }

    std::string GetHeader(const std::string &key)
    {
        auto iter = _headers.find(key);
        if (iter != _headers.end())
        {
            return iter->second;
        }
        return "";
    }

    void SetContent(const std::string& content, const std::string& type)
    {
        _body = content;
        SetHeader("Content-Length", std::to_string(content.size()));
        SetHeader("Content-Type", type);
    }

    void SetRedirect(std::string& url, int status = 302)
    {
        _status   = status;
        _redirect = true;
        _reurl    = url;
        SetHeader("Location", url);
    }

    // 创建一个 HTTP 响应报文
    void InitMsg(std::string& msg)
    {
        std::ostringstream responseStream;
        // 版本 状态码 状态码描述
        responseStream << "HTTP/1.1 " << _status << " " << Util::getHttpStatusDescription(_status) << DefaultLineSep;
        // 头部信息
        for (auto& header : _headers)
        {
            responseStream << header.first << DefaultHeaderSep << header.second << DefaultLineSep;
        }
        // 空行 + 正文
        responseStream << DefaultLineSep << _body;

        msg = responseStream.str();
    }

    // 设置状态码
    void SetStatus(int status) { _status = status; }

private:
    int          _status;        // 状态码
    bool         _redirect;      // 重定向标志
    std::string  _reurl;         // 重定向 url
    std::string  _body;          // 响应正文
    HeaderMap    _headers;       // 响应头
};

enum HttpRecvStatus
{
    LINE,
    HEAD,
    BODY,
    OVER,
    ERROR
};

class HttpContext
{
private:
    void RecvHttpLine(BufferPtr buffer)
    {
        if (_recv_status != LINE) { return; }
        
        std::string line = buffer->PeekLine(); 
        Util::RemoveNewline(line);
        if (line == "Line Empty...") 
        {
            spdlog::info(""); 
            return; 
        }
        ParseHttpLine(line);
        buffer->Update(); // 处理好了才能取
        _recv_status = HttpRecvStatus::HEAD;
        spdlog::info("Successful parse httpline = {} {} {}...", _request._method , _request._path, _request._version);
    }

    // GET /.../index?user=..&passward=.. HTTP/1.1
    void ParseHttpLine(const std::string& line)
    {
        std::vector<std::string> str;
        Util::Split(line, " ", str);
        // 获取方式
        if (line.size() > 0) { _request._method = str[0]; }
        // 获取版本
        if (line.size() > 2) { _request._version = str[2]; }
        // 获取资源路径和参数
        if (line.size() > 1)
        {
            std::string path_params = str[1];
            int pos = path_params.find("?");
            // 没有参数
            if (pos == std::string::npos) 
            {
                _request._path = path_params;
                return;
            }
            else
            {
                _request._path = path_params.substr(0, pos);
                std::string params = path_params.substr(pos + 1);
                Util::SplitParams(params, _request._params);
            }
        }
    }

    // key: val\r\nkey: val\r\n\r\n...
    void RecvHttpHeader(BufferPtr buffer)
    {
        if (_recv_status != HEAD) { return; }

        while (true)
        {
            std::string header = buffer->ReadLine();
            // 头部字段不全
            if (header == "") { return; }
            // "\r\n" 表示头部结束
            if (header == "\r\n" || header == "\n") { break; }
            // 去除换行符
            Util::RemoveNewline(header);
            ParseHttpHeader(header);
        }
        _recv_status = HttpRecvStatus::BODY;
        spdlog::info("Successful parse head...");
    }

    void ParseHttpHeader(const std::string& header)
    {
        // 分割键值对
        size_t sep_pos = header.find(DefaultHeaderSep);
        if (sep_pos == std::string::npos) 
        {
            _status = 400; 
            return; 
        } 
        std::string key = header.substr(0, sep_pos);
        std::string value = header.substr(sep_pos + DefaultHeaderSep.size());

        // 插入到请求头中
        if (!key.empty() && !value.empty()) 
        { 
            _request._headers.insert(std::make_pair(key, value)); 
            return;
        }
        _status = 400; 
    }

    void RecvHttpBody(BufferPtr buffer)
    {
        if (_recv_status != BODY) { return; }

        size_t len = _request.GetContentLength();
        // 正文不存在
        if (len == 0) 
        {
            _recv_status = HttpRecvStatus::OVER; 
            return; 
        }

        // 需要的大小 = 实际大小 - 已经接受大小
        // 缓冲区的大小已经足够
        int need_size = len - _request._body.size();
        if (buffer->ReadableBytes() >= need_size)
        {
            char tmp[DefaultSize];
            buffer->Read(tmp, need_size);
            _request._body +=  tmp;
            _recv_status = HttpRecvStatus::OVER;
            return;
        }
        // 缓冲区的大小不足够，先取出来
        _request._body += buffer->ReadAllContent();
        spdlog::info(" Successful parse body...");
    } 

public:
    HttpContext()
        : _status(200)
        , _recv_status(LINE)
    {}

    int RepStatus() { return _status; }

    int RecvStatus() { return _recv_status; }

    HttpRequest& GetRequest() { return _request; }

    void ParseHttpRequest(BufferPtr buffer)
    {
        switch (_recv_status)
        {
        case HttpRecvStatus::LINE:
            RecvHttpLine(buffer);
        case HttpRecvStatus::HEAD:
            RecvHttpHeader(buffer);
        case HttpRecvStatus::BODY:
            RecvHttpBody(buffer);
        }
    }

    void Clear()
    {
        _status = 200;
        _recv_status = HttpRecvStatus::LINE;
        _request.Clear();
    }

private: 
    int             _status; 
    HttpRecvStatus  _recv_status;
    HttpRequest     _request;
};
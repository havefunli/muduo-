#pragma once
#include "../include/TcpServer.h"
#include "../src/Util.hpp"
#include <string>
#include <unordered_map>

class HttpRequests;

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

    size_t GetContentLength()
    {
        auto iter = _headers.find("Content-Length");
        if (iter != _headers.end())
        {
            return std::stoi(GetHeader("Content-Length"));
        }
        return -1;
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

    void SetContent(const std::string& content, std::string& type)
    {
        _body = content;
        SetHeader("Content-Type", type);
    }

    void SetRedirect(std::string& url, int status = 302)
    {
        _status   = status;
        _redirect = true;
        _reurl    = url;
    }

private:
    int _status;        // 状态码
    bool _redirect;     // 重定向标志
    std::string _reurl; // 重定向 url
    std::string _body;  // 响应正文
    HeaderMap _headers; // 响应头
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
    void RecvHttpLine()
    {
        int pos = _buffer.find(DefaultLineSep);
        if (pos == std::string::npos) { return; }
        ParseHttpLine(_buffer.substr(0, pos));
        
        // buffer 去除首行
        _buffer = _buffer.substr(pos + DefaultLineSep.size());
        _recv_status = HttpRecvStatus::LINE;
    }

    // GET /.../index?user=..&passward=.. HTTP/1.1
    void ParseHttpLine(const std::string& line)
    {
        std::vector<std::string> str(3);
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
                ParamsMap parmap;
                Util::SplitParams(params, parmap);
                _request._params = parmap;
            }
        }
    }

    // key: val\r\nkey: val\r\n\r\n...
    void RecvHttpHeader()
    {
        while (true)
        {
            int pos = _buffer.find(DefaultLineSep);
            if (pos == std::string::npos) { return; } // 等待更多数据

            // 提取一行并更新缓冲区
            std::string header = _buffer.substr(0, pos);
            _buffer = _buffer.substr(pos + DefaultLineSep.size());

            // 空行表示头部结束
            if (header.empty()) { break; }

            ParseHttpHeader(header);
        }
        _recv_status = HttpRecvStatus::HEAD;
    }

    void ParseHttpHeader(const std::string& header)
    {
        // 空行表示头部结束
        if (header.empty()) { return; }

        // 分割键值对
        size_t sep_pos = header.find(DefaultHeaderSep);
        if (sep_pos == std::string::npos) { return; } 
        std::string key = header.substr(0, sep_pos);
        std::string value = header.substr(sep_pos + DefaultHeaderSep.size());

        // 插入到请求头中
        _request._headers.insert(std::make_pair(key, value));
    }

    void RecvHttpBody()
    {
        size_t len = _request.GetContentLength();
        if (len <= 0) 
        {
            _recv_status = HttpRecvStatus::OVER; 
            return; 
        }

        // 需要的大小 = 实际大小 - 已经接受大小
        int need_size = len - _request._body.size();
        if (_buffer.size() >= need_size)
        {
            _request._body += _buffer.substr(0, need_size);
            _buffer.substr(need_size);
            _recv_status = HttpRecvStatus::BODY;
            return;
        }
        
        _request._body += _buffer;
        _recv_status = HttpRecvStatus::OVER;
    } 

public:
    HttpContext()
        : _status(200)
        , _recv_status(LINE)
        , _buffer()
    {}

    void AppendContext(BufferPtr buff) { _buffer += buff->ReadAllContent(); }

    int ResStatus()
    {
         
    }

private:
    int             _status;
    std::string     _buffer;  
    HttpRecvStatus  _recv_status;
    HttpRequest     _request;
};
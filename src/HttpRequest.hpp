#pragma once
#include <string>
#include <unordered_map>

class HttpRequests;

using HeaderMap = std::unordered_map<std::string, std::string>;
using ParamsMap = std::unordered_map<std::string, std::string>;


class HttpRequest
{
public:
    std::string _method;  // 请求方法
    std::string _path;    // 资源路径
    std::string _version; // 协议版本
    std::string _body;    // 正文内容
    HeaderMap   _headers; // 请求头


    bool HashHeader(const std::string& key)
    {
        auto iter = _headers.find(key);
        return iter != _headers.end();
    }

    void SetHeader(const std::string& key, const std::string& val)
    {
        if (_headers.find(key) == _headers.end()) { _headers.insert(std::make_pair(key, val)); }
    }

    std::string GetHeader(const std::string& key)
    {
        auto iter = _headers.find(key);
        if (iter != _headers.end()) { return iter->second; }
        return "";
    }

    size_t GetContentLength()
    {
        auto iter = _headers.find("Content-Length");
        if (iter != _headers.end()) { return std::stoi(GetHeader("Content-Length")); }
        return -1;
    }
};
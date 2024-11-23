#pragma once
#include "../include/TcpServer.h"
#include <fstream>

class Util
{
public:
    /*字符串分割*/
    static void Split(const std::string& src, const std::string& sep, std::vector<std::string>& result)
    {
        int offset = 0;

        while (true)
        {
            int pos = src.find(sep, offset);
            if (pos == std::string::npos)
            {
                result.emplace_back(src.substr(offset));
                break;
            }
            
            result.emplace_back(src.substr(offset, pos - offset));
            offset = pos + sep.size();
        }
    }

    /*读取文件内容到 Out 中*/
    static void ReadFile(const std::string& path, std::string& out)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs.is_open())
        {
            spdlog::error("Fail open {}", path);
            return;
        }
        // 获取文件大小
        ifs.seekg(0, ifs.end);
        int len = ifs.tellg();
        ifs.seekg(0, ifs.beg);
        
        // 读取操作
        out.resize(len);
        ifs.read(&out[0], len);
        return;
    }

    /*写入内容到文件中*/
    static void WriteFile(const std::string& path, std::string& in)
    {
        std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
        {
            spdlog::error("Fail open {}", path);
            return;
        }
        
        ofs.write(in.c_str(), in.size());
        return;
    }
    
    /*将查询字符串中的特殊字符转义，避免和HTTP请求产生歧义*/
    /*某些特殊字符不进行转义 . - _ ~，空格看具体需求 */
    static void UrlEncode(const std::string& content, std::string& out, bool convert_space_to_plus)
    {
        for (auto ch : content)
        {
            if (ch == '.' || ch == '-' || ch == '_' || ch  == '~') { out += ch; }
            // 数字字母
            else if (isalnum(ch)) { out += ch; }
            // 空格 
            else if (ch == ' ' && convert_space_to_plus ) { out += '+'; }
            // 转义操作
            else
            {
                char temp[4] = {0};
                snprintf(temp, 4, "%%%02X", ch); // %% -> %; %02X -> 转化为大写的16进制（不足补0）
                out += temp;
            }
        }
    }

    static int HEXTOI(char c)
    {
        if (c >= '0' && c <= '9') { return c - '0'; }
        else if (c >= 'a' && c <= 'z') { return c - 'a' + 10 ; } // b - a + 10 = 11 
        else if (c >= 'A' && c <= 'Z') { return c - 'A' + 10 ; }
        else { return -1; }
    }

    /*将内容解码*/
    static void UrlDecode(const std::string& code, std::string& out, bool convert_space_to_plus)
    {
        for (int i = 0; i < code.size(); i++)
        {
            if (code[i] == '%')
            {
                char c = HEXTOI(code[i + 1]) * 16 + HEXTOI(code[i + 2]);
                out += c;
                i += 2;
            }
            else if (code[i] == '+' && convert_space_to_plus) { out += ' '; }
            else{ out += code[i]; }
        }
    }

    /*判断请求路径是否有效*/
    static bool IsValidPath(const std::string& path)
    {
        std::vector<std::string> sub_dir;
        Split(path, "/", sub_dir);

        int level = 0;
        for (auto& dir : sub_dir)
        {
            if (dir == "..") { level--; }
            else { level++; }

            if (level < 0) { return false; }
        }

        return true;
    }
    
    // 分离参数
    static void SplitParams(const std::string params, std::unordered_map<std::string, std::string>& paramsMap)
    {
        std::vector<std::string> paramsArray;
        Split(params, "&", paramsArray);
        for (int i = 0; i < paramsArray.size(); i++)
        {
            std::vector<std::string> param;
            Split(paramsArray[i], "=", param);
            if (param.size() < 1) { continue; }
            paramsMap.insert(std::make_pair(param[0], param[1]));
        }
    }
    
    // 输入状态码输出描述
    static std::string getHttpStatusDescription(int statusCode) 
    {
        switch (statusCode) {
            // 1xx Informational
            case 100: return "Continue";
            case 101: return "Switching Protocols";
            case 102: return "Processing";
            
            // 2xx Success
            case 200: return "OK";
            case 201: return "Created";
            case 202: return "Accepted";
            case 203: return "Non-Authoritative Information";
            case 204: return "No Content";
            case 205: return "Reset Content";
            case 206: return "Partial Content";
            case 207: return "Multi-Status";

            // 3xx Redirection
            case 300: return "Multiple Choices";
            case 301: return "Moved Permanently";
            case 302: return "Found";
            case 303: return "See Other";
            case 304: return "Not Modified";
            case 305: return "Use Proxy";
            case 307: return "Temporary Redirect";

            // 4xx Client Error
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 406: return "Not Acceptable";
            case 408: return "Request Timeout";
            case 409: return "Conflict";

            // 5xx Server Error
            case 500: return "Internal Server Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 504: return "Gateway Timeout";

            // Default for unknown codes
            default: return "Unknown Status Code";
        }
    }
};
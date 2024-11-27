#include "../include/Util.h"
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

// 定义常见扩展名和 MIME 类型映射
const std::unordered_map<std::string, std::string> Util::mimeTypes = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"css", "text/css"},
    {"js", "application/javascript"},
    {"json", "application/json"},
    {"xml", "application/xml"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"gif", "image/gif"},
    {"bmp", "image/bmp"},
    {"svg", "image/svg+xml"},
    {"ico", "image/x-icon"},
    {"webp", "image/webp"},
    {"pdf", "application/pdf"},
    {"txt", "text/plain"},
    {"csv", "text/csv"},
    {"mp3", "audio/mpeg"},
    {"wav", "audio/wav"},
    {"mp4", "video/mp4"},
    {"avi", "video/x-msvideo"},
    {"mov", "video/quicktime"},
    {"zip", "application/zip"},
    {"rar", "application/x-rar-compressed"},
    {"tar", "application/x-tar"},
    {"gz", "application/gzip"},
    {"exe", "application/octet-stream"},
    {"doc", "application/msword"},
    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {"xls", "application/vnd.ms-excel"},
    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {"ppt", "application/vnd.ms-powerpoint"},
    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"}};

void Util::Split(const std::string &src, const std::string &sep, std::vector<std::string> &result)
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

void Util::ReadFile(const std::string &path, std::string &out)
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
}

void Util::WriteFile(const std::string &path, std::string &in)
{
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
    {
        spdlog::error("Fail open {}", path);
        return;
    }

    ofs.write(in.c_str(), in.size());
}

void Util::UrlEncode(const std::string &content, std::string &out, bool convert_space_to_plus)
{
    for (auto ch : content)
    {
        if (ch == '.' || ch == '-' || ch == '_' || ch == '~')
        {
            out += ch;
        }
        // 数字字母
        else if (isalnum(ch))
        {
            out += ch;
        }
        // 空格
        else if (ch == ' ' && convert_space_to_plus)
        {
            out += '+';
        }
        // 转义操作
        else
        {
            char temp[4] = {0};
            snprintf(temp, 4, "%%%02X", ch); // %% -> %; %02X -> 转化为大写的16进制（不足补0）
            out += temp;
        }
    }
}

int Util::HEXTOI(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 10;
    }
    else
    {
        return -1;
    }
}

void Util::UrlDecode(const std::string &code, std::string &out, bool convert_space_to_plus)
{
    for (int i = 0; i < code.size(); i++)
    {
        if (code[i] == '%')
        {
            char c = HEXTOI(code[i + 1]) * 16 + HEXTOI(code[i + 2]);
            out += c;
            i += 2;
        }
        else if (code[i] == '+' && convert_space_to_plus)
        {
            out += ' ';
        }
        else
        {
            out += code[i];
        }
    }
}

bool Util::IsValidPath(const std::string &path)
{
    std::vector<std::string> sub_dir;
    Split(path, "/", sub_dir);

    int level = 0;
    for (auto &dir : sub_dir)
    {
        if (dir == "..")
        {
            level--;
        }
        else
        {
            level++;
        }

        if (level < 0)
        {
            return false;
        }
    }

    return true;
}

void Util::SplitParams(const std::string params, std::unordered_map<std::string, std::string> &paramsMap)
{
    std::vector<std::string> paramsArray;
    Split(params, "&", paramsArray);
    for (int i = 0; i < paramsArray.size(); i++)
    {
        std::vector<std::string> param;
        Split(paramsArray[i], "=", param);
        if (param.size() < 1)
        {
            continue;
        }
        paramsMap.insert(std::make_pair(param[0], param[1]));
    }
}

std::string Util::getHttpStatusDescription(int statusCode)
{
    switch (statusCode)
    {
    // 1xx Informational
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";
    case 102:
        return "Processing";

    // 2xx Success
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 207:
        return "Multi-Status";

    // 3xx Redirection
    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 307:
        return "Temporary Redirect";

    // 4xx Client Error
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";

    // 5xx Server Error
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";

    // Default for unknown codes
    default:
        return "Unknown Status Code";
    }
}

std::string Util::GetMimeType(const std::string &path)
{
    auto iter = path.find_last_of('.');
    if (iter == std::string::npos)
    {
        return "application/octet-stream";
    }

    std::string extension = path.substr(iter + 1);

    // 查找扩展名对应的 MIME 类型
    auto it = mimeTypes.find(extension);
    return it != mimeTypes.end() ? it->second : "application/octet-stream";
}

bool Util::IsFileSrc(const std::string &path)
{
    if (path.empty())
    {
        return false;
    }
    // 找到后缀名
    auto iter = path.find_last_of('.');
    if (iter == std::string::npos)
    {
        return false;
    }
    // 判断是否存在这个文件类型
    std::string extension = path.substr(iter + 1);
    return mimeTypes.find(extension) != mimeTypes.end();
}

void Util::RemoveNewline(std::string &str)
{
    if (str.empty())
    {
        return;
    }
    size_t pos = str.find("\r\n");
    if (pos != std::string::npos)
    {
        str.erase(pos, 2);
    } // 删除 `\r\n`
    else
    {
        str.erase(str.find("\r"));
    } // 删除 `\n`
}
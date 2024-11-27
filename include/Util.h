#ifndef UTIL_H
#define UTIL_H
#pragma once

#include "TcpServer.h"
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>

class Util
{
public:
    /* 字符串分割 */
    static void Split(const std::string &src, const std::string &sep, std::vector<std::string> &result);

    /* 读取文件内容到 Out 中 */
    static void ReadFile(const std::string &path, std::string &out);

    /* 写入内容到文件中 */
    static void WriteFile(const std::string &path, std::string &in);

    /* 将查询字符串中的特殊字符转义，避免和HTTP请求产生歧义 */
    static void UrlEncode(const std::string &content, std::string &out, bool convert_space_to_plus);

    static int HEXTOI(char c);

    /* 将内容解码 */
    static void UrlDecode(const std::string &code, std::string &out, bool convert_space_to_plus);

    /* 判断请求路径是否有效 */
    static bool IsValidPath(const std::string &path);

    // 分离参数
    static void SplitParams(const std::string params, std::unordered_map<std::string, std::string> &paramsMap);

    // 输入状态码输出描述
    static std::string getHttpStatusDescription(int statusCode);

    static std::string GetMimeType(const std::string &path);

    static bool IsFileSrc(const std::string &path);

    static void RemoveNewline(std::string &str);

    // 定义常见扩展名和 MIME 类型映射
    static const std::unordered_map<std::string, std::string> mimeTypes;
};

#endif // UTIL_H

#include "../src/Http.hpp"

void test_1()
{
    std::string msg =
        "GET /path/resource HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: CustomHttpClient/1.0\r\n"
        "Content-Length: 24\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Connection: keep-alive\r\n\r\n"
        "Hello, World!"; // 空行结束请求头

    BufferPtr buffer(new Buffer());
    buffer->Write(msg);

    HttpContext context;
    context.ParseHttpRequest(buffer);
    HttpRequest req = context.GetRequest();
    std::cout << req._method << " " << req._path << " " << req._version << std::endl;
    for (auto& pair : req._headers)
    {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    std::cout << req._body << std::endl;
}

int main()
{
    test_1();
    return 0;
}
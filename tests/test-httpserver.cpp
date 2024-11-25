#include "../include/HttpServer.h"

int main()
{
    HttpServer server(8888, 2);
    server.SetBaseDir("./wwwroot/");
    server.Start();
    return 0;
}
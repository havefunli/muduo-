#include "../src/Socket.hpp"
#include <iostream>


int main()
{
    std::unique_ptr<Socket> ListenPtr = std::make_unique<TcpSocket>();
    ListenPtr ->BuildTcpListen(8888);

    SocketAddress client;
    int code = 0;
    int fd = ListenPtr->AcceptConnect(client, code);

    std::unique_ptr<Socket> ClientPtr = std::make_unique<TcpSocket>(fd);
    while (true)
    {
        std::string buffer;
        ClientPtr->Recv(buffer);
        std::cout << buffer;
        sleep(2);
    }

    return 0;
}
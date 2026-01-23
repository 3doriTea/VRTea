#include "NetQueue.h"

void NetQueue::Send(std::string content)
{
    sendQueue.push(content);
}

std::string NetQueue::Read()
{
    std::string Q = readQueue.front();
    readQueue.pop();

    return Q;
}

NetQueue::NetQueue()
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        WSACleanup();
        return;
    }
    set_nonblocking(sock);
}

NetQueue::~NetQueue()
{

}

void NetQueue::Update()
{
    std::string sendData;

    int ret = send(sock, 
        sendData.c_str(),
        (sendData.size()),  // ‘—‚é•¶Žš—ñ‚ÌƒTƒCƒY
        0);                   
}

void NetQueue::Drow()
{

}

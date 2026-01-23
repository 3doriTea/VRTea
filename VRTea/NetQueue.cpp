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

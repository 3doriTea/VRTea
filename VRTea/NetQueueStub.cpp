#include "NetQueueStub.h"
#include <nlohmann/json.hpp>
#include <random>
#include "GameTime.h"
namespace
{
    const int MAX_STR_LEN{10};
    std::random_device rd;
    std::mt19937 mt(rd());
    float readDuration = 1.0f;
    float elapsed = 0.0f;

    std::string GenContent()
    {
        std::string content;


        std::uniform_int_distribution<> lenDist(1, MAX_STR_LEN);
        int len = lenDist(rd);
        for (int i = 0; i < len;i++)
        {
            std::uniform_int_distribution<> dist(0, 'z' - 'a');
            int result = dist(rd);
            char c = 'a' + result;
            content += c;
        }

        return content;
    }
}
void NetQueueStub::Send(std::string content)
{
    sendQueue.push(content);
}

std::string NetQueueStub::Read()
{
    if (readQueue.empty())
        return "";

    std::string Q = readQueue.front();
    readQueue.pop();

    return Q;
}

json NetQueueStub::Find(const std::string& tag)
{
    if (readQueue.empty())
        return "";

    std::string Q = readQueue.front();
    readQueue.pop();

    return Q;
}

NetQueueStub::NetQueueStub()
{
    nlohmann::json datas;
    datas["Head"] = "Event";
    datas["Content"] = "uwaaaa";
    std::string str = datas.dump();
    readQueue.push(str);

    datas["Head"] = "Event";
    datas["Content"] = "apple";
    str = datas.dump();
    readQueue.push(str);
}

NetQueueStub::~NetQueueStub()
{

}

void NetQueueStub::Update()
{
    elapsed += GameTime::DeltaTime();
    /*if (elapsed >= readDuration)
    {
        elapsed = 0.0f;
        nlohmann::json j;
        j["Head"] = "Event";
        j["Content"] = GenContent();
        readQueue.push(j.dump());
    }*/
}


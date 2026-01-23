#pragma once
#define WIN32_LEAN_AND_MEAN
#include "DxLib.h"
#include "GameObject.h"
#include <queue>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <unordered_map>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

struct NetQueue : GameObject
{
NetQueue();
~NetQueue();

void Update() override;
void Drow();

void Send(std::string content);
std::string Read();

std::queue<std::string> sendQueue;
std::queue<std::string> readQueue;

SOCKET sock;  // ソケット作成
};
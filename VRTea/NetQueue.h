#pragma once
#include "GameObject.h"
#include <queue>

struct NetQueue : GameObject
{
void Send(std::string content);
std::string Read();

std::queue<std::string> sendQueue;
std::queue<std::string> readQueue;
};
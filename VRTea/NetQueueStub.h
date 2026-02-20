#pragma once
#include "GameObject.h"
#include <queue>
#include <vector>
#include <cstring>


struct NetQueueStub : GameObject
{
	NetQueueStub();
	~NetQueueStub();

	void Update() override;

	void Send(std::string content);
	std::string Read();

	std::queue<std::string> sendQueue;
	std::queue<std::string> readQueue;
};
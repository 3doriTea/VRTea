#pragma once
#include <chrono>

class GameTime
{
public:
	static  void Initialize();
	static  void Refresh();
	static  float DeltaTime();

private:
	GameTime() = default;
	GameTime(GameTime& _other) = delete;
	GameTime(GameTime&& _other) = delete;
	static GameTime instance_;
	std::chrono::high_resolution_clock::time_point prevTime;
	std::chrono::high_resolution_clock::time_point currTime;
	float deltaTime;
};
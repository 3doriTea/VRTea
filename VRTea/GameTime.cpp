#include "GameTime.h"

GameTime GameTime::instance_;
using namespace std::chrono;

void GameTime::Initialize()
{
	GameTime& instance = GameTime::instance_;
	instance.prevTime = high_resolution_clock::now();
	instance.currTime = GameTime::instance_.prevTime;
}
void GameTime::Refresh()
{
	GameTime& instance = GameTime::instance_;

	instance.currTime = high_resolution_clock::now();
	long long deltaTimeMillSec = duration_cast<std::chrono::milliseconds>(instance.currTime - instance.prevTime).count();
	instance.deltaTime = static_cast<float>(deltaTimeMillSec) / 1000.0f;
	instance.prevTime = instance.currTime;
}

float GameTime::DeltaTime()
{
	return GameTime::instance_.deltaTime;
}



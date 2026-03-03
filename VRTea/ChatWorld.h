#pragma once
#include "IncludingWin.h"
#include "GameObject.h"

struct ChatWorld : GameObject
{
	ChatWorld();
	~ChatWorld();
	void Update() override;
	void Draw() override;
};
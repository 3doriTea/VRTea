#pragma once
#include "GameObject.h"

struct OtherPlayer : GameObject
{
	OtherPlayer();
	~OtherPlayer();
	void Update() override;
	void Draw() override;
};
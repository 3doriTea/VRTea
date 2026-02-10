#pragma once
#include "GameObject.h"
#include "IncludingWin.h"

struct OtherPlayer : GameObject
{
	OtherPlayer();
	~OtherPlayer();
	void Update() override;
	void Draw() override;

	void SetPosition(VECTOR pos);
};
#pragma once
#include "GameObject.h"
#include <DxLib.h>

struct PlayerCamera : GameObject
{
	void Update() override;
	void Draw() override;

	void SetPosition(VECTOR pos);
	float angleX, angleY;
};
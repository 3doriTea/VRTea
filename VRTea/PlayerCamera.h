#pragma once
#include "GameObject.h"

struct PlayerCamera : GameObject
{
	PlayerCamera();
	~PlayerCamera();
	void Update() override;
	void Draw() override;
	
	void SetPosition(VECTOR pos);

	float GetAngleY() const;

private:
	float angleX, angleY;
	VECTOR camPos;
	VECTOR target;
};
#include "PlayerCamera.h"

void PlayerCamera::Update()
{
	SetCameraPositionAndTargetAndUpVec(VGet(0.0f,0.0f,-10.0f), VGet(0.0f, 0.0f, 0.0f),);
}

void PlayerCamera::Draw()
{
	//DrawCapsule3D();
}

void PlayerCamera::SetPosition(VECTOR pos)
{
}

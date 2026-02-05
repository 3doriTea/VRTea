#include "PlayerCamera.h"

PlayerCamera::PlayerCamera()
{
	SetUseLighting(FALSE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
}

PlayerCamera::~PlayerCamera()
{
}

void PlayerCamera::Update()
{
	//SetCameraPositionAndTargetAndUpVec(VGet(0.0f,0.0f,-10.0f),VGet(0.0f, 0.0f,0.0f),VGet(0.0f,1.0f,0.0f));
	int mouseX = 0, mouseY = 0;
	int prevX = 0, prevY = 0;
	GetMousePoint(&mouseX, &mouseY);
	//angleX = mouseX - prevX;
	//angleY = mouseY - prevY;
	//prevX = mouseX;
	//prevY = mouseY;
	//VECTOR target;




	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 0.0f, -100.0f), VGet(0.0f, 0.0f, 0.0f));

	prevX = mouseX;
	prevY = mouseY;
}

void PlayerCamera::Draw()
{
	DrawCapsule3D(VGet(0.0f,0.0f,0.0f),VGet(0.0f,10.0f,0.0f),2.0f,20.0f,GetColor(255,0,0),GetColor(255,255,255),TRUE);
	DrawCapsule3D(VGet(0.0f, 0.0f, -300.0f), VGet(0.0f, 10.0f, -300.0f), 2.0f, 20.0f, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);
	DrawLine3D(VGet(-100.0f, 0.0f, 0.0f), VGet(100.0f, 0.0f, 0.0f), GetColor(255, 255, 255));
	//DrawCircle(100, 100, 10.0f, GetColor(0, 255, 0), TRUE);

	//DrawCapsule3D(VGet(0.0f, 0.0f, 0.0f), VGet(0.0f, 10.0f, 0.0f), 40.0f, 8, GetColor(0, 255, 0), GetColor(255, 255, 255), TRUE);
}

void PlayerCamera::SetPosition(VECTOR pos)
{
}

#include "PlayerCamera.h"

PlayerCamera::PlayerCamera()
{
	SetUseLighting(FALSE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	angleX = 0.0f;
	angleY = 0.0f;
	camPos = VGet(0.0f, 10.0f, -100.0f);
}

PlayerCamera::~PlayerCamera()
{
}

void PlayerCamera::Update()
{
#if false//デバッグ用(位置を動かすもの)
	if (CheckHitKey(KEY_INPUT_A))
	{
		camPos.x -= 3.0f;
	}
	if (CheckHitKey(KEY_INPUT_D))
	{
		camPos.x += 3.0f;
	}
	if (CheckHitKey(KEY_INPUT_W))
	{
		camPos.z += 3.0f;
	}
	if (CheckHitKey(KEY_INPUT_S))
	{
		camPos.z -= 3.0f;
	}
	if (CheckHitKey(KEY_INPUT_UP))
	{
		camPos.y += 3.0f;
	}
	if (CheckHitKey(KEY_INPUT_DOWN))
	{
		camPos.y -= 3.0f;
	}
#endif
	int mouseX = 0, mouseY = 0;
	int width = 1280, height = 720;
	float mouseSpeedX = 0.001f, mouseSpeedY = 0.001f;
	bool isKey = false;
	bool isSetPoint = true;
	GetMousePoint(&mouseX, &mouseY);
	int px = mouseX - width / 2.0f;
	int py = mouseY - height / 2.0f;
	//SetMousePoint(width / 2.0f, height / 2.0f);

	angleX += px * mouseSpeedX;
	angleY -= py * mouseSpeedY;

	if (angleY > 1.0f)
		angleY = 1.0f;
	if (angleY < -1.0f)
		angleY = -1.0f;

	target = VGet(0.0f, 0.0f, 0.0f);
	target.x = camPos.x + cos(angleY) * sin(angleX);
	target.y = camPos.y + sin(angleY);
	target.z = camPos.z + cos(angleY) * cos(angleX);

	SetCameraPositionAndTarget_UpVecY(camPos, target);
}

void PlayerCamera::Draw()
{
	//DrawCapsule3D(VGet(0.0f,0.0f,0.0f),VGet(0.0f,10.0f,0.0f),2.0f,20.0f,GetColor(255,0,0),GetColor(255,255,255),TRUE);
	//DrawCapsule3D(VGet(0.0f, 0.0f, -300.0f), VGet(0.0f, 10.0f, -300.0f), 2.0f, 20.0f, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);
	//DrawLine3D(VGet(-100.0f, 0.0f, 0.0f), VGet(100.0f, 0.0f, 0.0f), GetColor(255, 255, 255));
	//DrawLine3D(VGet(-100.0f, 100.0f, 0.0f), VGet(100.0f, -100.0f, 0.0f), GetColor(255, 255, 255));
	//DrawCircle(100, 100, 10.0f, GetColor(0, 255, 0), TRUE);

	//DrawCapsule3D(VGet(0.0f, 0.0f, 0.0f), VGet(0.0f, 10.0f, 0.0f), 40.0f, 8, GetColor(0, 255, 0), GetColor(255, 255, 255), TRUE);
}

void PlayerCamera::SetPosition(VECTOR pos)
{
	camPos = pos;
}

void PlayerCamera::GetAngle(VECTOR angle)
{
	angle = target;
}

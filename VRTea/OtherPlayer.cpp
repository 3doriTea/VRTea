#include "OtherPlayer.h"

OtherPlayer::OtherPlayer()
{
}

OtherPlayer::~OtherPlayer()
{
}

void OtherPlayer::Update()
{
}

void OtherPlayer::Draw()
{
	int boxSizeX = 128, boxSizeY = 128;
	VECTOR capsulePos = VGet(0.0f, 0.0f, 0.0f);
	VECTOR textBoxPos = VGet(0.0f, 0.0f, 0.0f);
	VECTOR trianglePos = VGet(0.0f, 0.0f, 0.0f);
	textBoxPos = ConvWorldPosToScreenPos(VGet(capsulePos.x, capsulePos.y + 16.0f, capsulePos.z));
	trianglePos = ConvWorldPosToScreenPos(VGet(capsulePos.x, capsulePos.y + 12.0f, capsulePos.z));

	int triangleSizeX = boxSizeX / 2.0f;
	int triangleSizeY = textBoxPos.y;

	if (textBoxPos.z <= 1.0f)
	{
		//メッセージボックスの表示にDrawBoxとDrawTriangleを使う
		DrawBox((textBoxPos.x - boxSizeX) - 3, (textBoxPos.y - boxSizeY) - 3, (textBoxPos.x + boxSizeX) + 3, textBoxPos.y + 3, GetColor(0, 0, 255), TRUE);
		DrawTriangle(trianglePos.x, (trianglePos.y) + 3, (trianglePos.x - triangleSizeX) - 3, triangleSizeY, (trianglePos.x + triangleSizeX) + 3, triangleSizeY, GetColor(0, 0, 255), TRUE);
		DrawBox(textBoxPos.x - boxSizeX, textBoxPos.y - boxSizeY, textBoxPos.x + boxSizeX, textBoxPos.y, GetColor(255, 255, 255), TRUE);
		DrawTriangle(trianglePos.x, trianglePos.y, (trianglePos.x - triangleSizeX), triangleSizeY, (trianglePos.x + triangleSizeX), triangleSizeY, GetColor(255, 255, 255), TRUE);
	}

	DrawCapsule3D(capsulePos, VGet(capsulePos.x, capsulePos.y + 10.0f, capsulePos.z), 2.0f, 20.0f, GetColor(0, 0, 0), GetColor(128,128,128), TRUE);
	//DrawCapsule3D(VGet(0.0f, 0.0f, -300.0f), VGet(0.0f, 10.0f, -300.0f), 2.0f, 20.0f, GetColor(0, 0, 0), GetColor(255, 255, 255), TRUE);
	DrawLine3D(VGet(-100.0f, 0.0f, 0.0f), VGet(100.0f, 0.0f, 0.0f), GetColor(255, 255, 255));
	DrawLine3D(VGet(-100.0f, 100.0f, 0.0f), VGet(100.0f, -100.0f, 0.0f), GetColor(255, 255, 255));
	//DrawCircle(100, 100, 10.0f, GetColor(0, 255, 0), TRUE);

	//DrawCapsule3D(VGet(0.0f, 0.0f, 0.0f), VGet(0.0f, 10.0f, 0.0f), 40.0f, 8, GetColor(0, 255, 0), GetColor(255, 255, 255), TRUE);
}

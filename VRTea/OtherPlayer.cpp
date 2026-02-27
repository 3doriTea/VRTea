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
	int boxSizeX = 128, boxSizeY = 128;//テキストボックスのサイズ
	float capsuleHeight = 10.0f;//カプセルの高さ
	float capsuleWidth = 2.0f;//カプセルの幅
	float triangleHeight = capsuleHeight + capsuleWidth;//頂点の位置
	float textBoxHeight = triangleHeight + 4.0f;//テキストボックスの高さ調整
	VECTOR capsulePos = VGet(0.0f, 0.0f, 0.0f);//カプセルの位置
	VECTOR textBoxPos = VGet(0.0f, 0.0f, 0.0f);//テキストボックスの位置
	VECTOR trianglePos = VGet(0.0f, 0.0f, 0.0f);
	textBoxPos = ConvWorldPosToScreenPos(VGet(capsulePos.x, capsulePos.y + textBoxHeight, capsulePos.z));
	trianglePos = ConvWorldPosToScreenPos(VGet(capsulePos.x, capsulePos.y + triangleHeight, capsulePos.z));

	int triangleSizeX = 10.0f;//幅
	int triangleSizeY = textBoxPos.y;

	if (textBoxPos.z <= 1.0f)
	{
		//テキストボックスの枠
		DrawBox((textBoxPos.x - boxSizeX) - 3, (textBoxPos.y - boxSizeY) - 3, (textBoxPos.x + boxSizeX) + 3, textBoxPos.y + 3, GetColor(0, 0, 255), TRUE);
		DrawTriangle(trianglePos.x, (trianglePos.y) + 3, (trianglePos.x - triangleSizeX) - 3, triangleSizeY, (trianglePos.x + triangleSizeX) + 3, triangleSizeY, GetColor(0, 0, 255), TRUE);
		//テキストボックス
		DrawBox(textBoxPos.x - boxSizeX, textBoxPos.y - boxSizeY, textBoxPos.x + boxSizeX, textBoxPos.y, GetColor(255, 255, 255), TRUE);
		DrawTriangle(trianglePos.x, trianglePos.y, (trianglePos.x - triangleSizeX), triangleSizeY, (trianglePos.x + triangleSizeX), triangleSizeY, GetColor(255, 255, 255), TRUE);
	}
	//カプセル
	DrawCapsule3D(capsulePos, VGet(capsulePos.x, capsulePos.y + capsuleHeight, capsulePos.z), capsuleWidth, 20.0f, GetColor(0, 0, 0), GetColor(128,128,128), TRUE);

	//デバッグ確認
	DrawLine3D(VGet(-100.0f, 0.0f, 0.0f), VGet(100.0f, 0.0f, 0.0f), GetColor(255, 255, 255));
	DrawLine3D(VGet(-100.0f, 100.0f, 0.0f), VGet(100.0f, -100.0f, 0.0f), GetColor(255, 255, 255));
}

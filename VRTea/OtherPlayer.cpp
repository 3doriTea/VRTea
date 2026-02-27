#include "OtherPlayer.h"
#include "NetQueue.h"



DxLib::COLOR_U8 GetColorFromJson(const json& _colorJson)
{
	unsigned int color;
	_colorJson.get_to(color);
	unsigned char* ptr = (unsigned char*)&color;
	unsigned char rgb[3];

	for (int i = 0; i < 3; i++)
	{
		rgb[2 - i] = *(ptr + sizeof(unsigned char) * i);
	}

	DxLib::COLOR_U8 resultColor;
	resultColor.r = rgb[0];
	resultColor.g = rgb[1];
	resultColor.b = rgb[2];
	return resultColor;
}

OtherPlayer::OtherPlayer()
	: otherPlayerCapsuleHeight_{10.0f}
	, otherPlayerCapsuleRadius_{5.0f}
	, otherPlayerCapsuleDivNum_{5}
{
}

OtherPlayer::~OtherPlayer()
{
}

void OtherPlayer::Update()
{
	json otherPlayerJson = FindGameObject<NetQueue>()->Find("Updated");
	otherPlayerJson.items();

	for (const auto& data : otherPlayerJson["content"].items())
	{
		OtherPlayerData otherPlayerData;

		// 名前
		otherPlayerData.name = data.key();

		// 座標
		DxLib::VECTOR position;
		json positionJson = data.value().at("position");
		position.x = positionJson.at("x").get<float>();
		position.y = positionJson.at("y").get<float>();
		position.z = positionJson.at("z").get<float>();
		otherPlayerData.position = position;

		// 色
		otherPlayerData.color = GetColorFromJson(data.value().at("color"));

		// 他プレイヤーのデータを格納
		otherPlayersData_.push_back(otherPlayerData);
	}

}

void OtherPlayer::Draw()
{	
	DrawLine3D(VGet(-100.0f, 0.0f, 0.0f), VGet(100.0f, 0.0f, 0.0f), GetColor(255, 255, 255));
	DrawLine3D(VGet(-100.0f, 100.0f, 0.0f), VGet(100.0f, -100.0f, 0.0f), GetColor(255, 255, 255));

	DrawOtherPlayer();
}

void OtherPlayer::DrawOtherPlayer()
{
	for (const auto& data : otherPlayersData_)
	{
		const DxLib::COLOR_U8& color = data.color;
		unsigned int colorCode = DxLib::GetColor(color.r, color.g, color.b);
		const DxLib::VECTOR& position = data.position;
		DrawCapsule3D(position, VGet(position.x, position.y + otherPlayerCapsuleHeight_, position.z), otherPlayerCapsuleRadius_, otherPlayerCapsuleDivNum_, colorCode, GetColor(255, 255, 255), TRUE);
		DrawMessageBox(position);
	}
	otherPlayersData_.clear();
}

void OtherPlayer::DrawMessageBox(const DxLib::VECTOR& playerPos)
{
	int boxSizeX = 128, boxSizeY = 128;
	VECTOR textBoxPos = VGet(0.0f, 0.0f, 0.0f);
	VECTOR trianglePos = VGet(0.0f, 0.0f, 0.0f);
	textBoxPos = ConvWorldPosToScreenPos(VGet(playerPos.x, playerPos.y + 16.0f, playerPos.z));
	trianglePos = ConvWorldPosToScreenPos(VGet(playerPos.x, playerPos.y + 12.0f, playerPos.z));

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
}

#include "OtherPlayer.h"
#include "NetQueue.h"


/// <summary>
/// JSONから色情報を取得
/// </summary>
/// <param name="colorJson"></param>
/// <returns></returns>
DxLib::COLOR_U8 GetColorFromJson(const json& colorJson)
{
	// 色情報を取得
	unsigned int color = colorJson.get<unsigned int>();
	// 先頭を指すポインタ
	unsigned char* ptr = (unsigned char*)&color;
	// rgbを一時的に格納する配列
	unsigned char rgb[3];

	// r,g,bを抽出
	for (int i = 0; i < 3; i++)
	{
		rgb[2 - i] = *(ptr + sizeof(unsigned char) * i);
	}

	// COLOR_U8に入れて返す
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
	//デバッグ確認
	DrawLine3D(VGet(-100.0f, 0.0f, 0.0f), VGet(100.0f, 0.0f, 0.0f), GetColor(255, 255, 255));
	DrawLine3D(VGet(-100.0f, 100.0f, 0.0f), VGet(100.0f, -100.0f, 0.0f), GetColor(255, 255, 255));

	DrawOtherPlayer();
}

void OtherPlayer::DrawOtherPlayer()
{
	for (const auto& data : otherPlayersData_)
	{
		// カラーコード取得
		const DxLib::COLOR_U8& color = data.color;
		unsigned int colorCode = DxLib::GetColor(color.r, color.g, color.b);

		// 座標取得
		const DxLib::VECTOR& position = data.position;

		// カプセル描画
		DrawCapsule3D(position, VGet(position.x, position.y + otherPlayerCapsuleHeight_, position.z), otherPlayerCapsuleRadius_, otherPlayerCapsuleDivNum_, colorCode, GetColor(255, 255, 255), TRUE);
		
		// メッセージボックス描画
		DrawMessageBox(position);
	}

	// 他プレイヤーの情報を捨てる
	otherPlayersData_.clear();
}

void OtherPlayer::DrawMessageBox(const DxLib::VECTOR& playerPos)
{
	int boxSizeX = 128, boxSizeY = 128;//テキストボックスのサイズ
	float capsuleHeight = 10.0f;//カプセルの高さ
	float capsuleWidth = 2.0f;//カプセルの幅
	float triangleHeight = capsuleHeight + capsuleWidth;//頂点の位置
	float textBoxHeight = triangleHeight + 4.0f;//テキストボックスの高さ調整
	VECTOR textBoxPos = VGet(0.0f, 0.0f, 0.0f);//テキストボックスの位置
	VECTOR trianglePos = VGet(0.0f, 0.0f, 0.0f);
	textBoxPos = ConvWorldPosToScreenPos(VGet(playerPos.x, playerPos.y + textBoxHeight, playerPos.z));
	trianglePos = ConvWorldPosToScreenPos(VGet(playerPos.x, playerPos.y + triangleHeight, playerPos.z));

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
}

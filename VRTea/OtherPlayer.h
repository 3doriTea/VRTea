#pragma once
#include "GameObject.h"
#include <vector>
struct OtherPlayer : GameObject
{
	OtherPlayer();
	~OtherPlayer();
	void Update() override;
	void Draw() override;
	/// <summary>
	/// 他プレイヤーを描画する
	/// </summary>
	void DrawOtherPlayer();
	/// <summary>
	/// 他プレイヤーの上にメッセージボックスを描画する
	/// </summary>
	/// <param name="playerPos">ボックス位置の基準にする他プレイヤー座標</param>
	void DrawMessageBox(const DxLib::VECTOR& playerPos);
	struct OtherPlayerData
	{
		DxLib::VECTOR position;
		DxLib::COLOR_U8 color;
		std::string name;
	};
	std::vector<OtherPlayerData> otherPlayersData_;
	// 他プレイヤーのカプセルの高さ
	float otherPlayerCapsuleHeight_;
	// 他プレイヤーのカプセルの半径
	float otherPlayerCapsuleRadius_;
	// 他プレイヤーのカプセルの分割数
	int otherPlayerCapsuleDivNum_;
};
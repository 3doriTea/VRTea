#pragma once
#include "GameObject.h"

struct OtherPlayerData
{
	inline OtherPlayerData(
		const std::string& _name,
		const VECTOR& _position,
		unsigned int _color) :
		name{ _name },
		position{ _position },
		color{ _color }
	{
	}

	std::string name;
	VECTOR position;
	unsigned int color;
};

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

private:
	std::vector<OtherPlayerData> data_;
};
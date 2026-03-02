#pragma once
#include "GameObject.h"
#include <unordered_map>
#include "Chat.h"
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

struct OtherPlayerChat
{
	float timeLeft;
	std::string content;
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
	void DrawMessageBox(const DxLib::VECTOR& playerPos,const std::string& sender);

private:
	void ChatEventHandler(const ChatContent& content);
	std::vector<OtherPlayerData> otherPlayersData_;
	std::unordered_map<std::string, OtherPlayerChat> otherPlayerChatMap_;
	// 他プレイヤーのカプセルの高さ
	float otherPlayerCapsuleHeight_;
	// 他プレイヤーのカプセルの半径
	float otherPlayerCapsuleRadius_;
	// 他プレイヤーのカプセルの分割数
	int otherPlayerCapsuleDivNum_;
	float otherPlayerChatDisplayTime_;
};
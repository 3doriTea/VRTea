#pragma once
#include <IncludingWin.h>
#include "PlayerData.h"
#include "PlayerState.h"
#include "GameObject.h"

struct Player : GameObject
{
	Player();

	void Update() override;
	void Draw() override;

	~Player();

	void ChangeName(const std::string& _name)
	{
		 pData.name = _name;
	};
	void ChangeColor(const DxLib::COLOR_U8 _color)
	{
		pData.color = _color;
	};
	void DrawImGui();
	PlayerData pData;
	PlayerState playerState;
};

#pragma once
#include "IncludingWin.h"
#include "../ImGui/imgui.h"
#include "PlayerData.h"
#include "PlayerState.h"
#include "GameObject.h"
#include "NetQueue.h"

struct Player : GameObject
{
	Player();

	void Update() override;
	void Draw() override;

	~Player();

//<<<<<<< HEAD
	void ChangeName(const std::string _name) { pData.name = _name; };
	void ChangeColor(const DxLib::COLOR_U8 _color) { pData.color = _color; };
	void PushPData(const json data);
//=======
	void ChangeNameImGui();
	void ChangeColorImGui();
	void TextChatImGui();
	void SetMyCamera();
	
//>>>>>>> d0721691511c3f8b0ef325c44ed50b89bbd7724f
	PlayerData pData;
	PlayerState playerState;
	const int SPEED = 4.0;
};

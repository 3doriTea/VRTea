#pragma once
#include "IncludingWin.h"
#include "../ImGui/imgui.h"
#include "PlayerData.h"
#include "PlayerState.h"
#include "GameObject.h"

struct Player : GameObject
{
	Player();

	void Update() override;
	void Draw() override;

	~Player();

	void ChangeName(const std::string _name) { pData.name = _name; };
	void ChangeColor(const DxLib::COLOR_U8 _color) { pData.color = _color; };
	void DrawImGui()
	{
		char* newName;
		ImGui::Begin("変更したいユーザー名を入力してください。");
		ImGui::InputText("new Name : ", newName, 20);
		ImGui::End();

		std::string name(newName);
		ChangeName(name);
	};
	PlayerData pData;
	PlayerState playerState;
};

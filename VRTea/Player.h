#pragma once
#include "../ImGui/imgui.h"
#include "PlayerData.h"
#include "PlayerState.h"
#include "GameObject.h"
#include "NetQueue.h"


namespace
{
	static const size_t NAME_SIZE_MAX = 20;
	static const size_t NAME_BUFFER_SIZE = NAME_SIZE_MAX + 1;  // null•¶Žš•ª’Ç‰Á

	enum
	{
		COLOR_AT_R,
		COLOR_AT_G,
		COLOR_AT_B,
		COLOR_AT_A,
	};
}

struct Player;

struct IPlayModeState
{
	inline IPlayModeState() {}
	inline virtual ~IPlayModeState() {}

	virtual void Enter(Player& _p) {}
	virtual void Update(Player& _p) = 0;
	virtual void Exit(Player& _p) {}
};

struct GameMode : IPlayModeState
{
	void Update(Player& _p) override;
};

struct MenuMode : IPlayModeState
{
	void Enter(Player& _p) override;
	void Update(Player& _p) override;
	void Exit(Player& _p) override;
};

struct ChatMode : IPlayModeState
{
	void Enter(Player& _p) override;
	void Update(Player& _p) override;
};

struct Player : GameObject
{
	Player();

	void Update() override;
	void Draw() override;

	~Player();

	void ChangeName(const std::string _name) { pData.name = _name; };
	void ChangeColor(const DxLib::COLOR_U8 _color) { pData.color = _color; };
	void PushPData(const json data);
	//void ChangeNameImGui();
	//void ChangeColorImGui();
	void TextChatImGui();
	void SetMyCamera();
	
	PlayerData pData;
	PlayerState playerState;
	const int SPEED = 4.0;

	bool prevMKeyPressed_;
	bool prevTKeyPressed_;

	char nameBuffer[NAME_BUFFER_SIZE];
	float colorBuffer[COLOR_AT_A];

	IPlayModeState* pPlayModeState_;
};

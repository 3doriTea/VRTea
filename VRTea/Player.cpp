#include "Player.h"
#include "PlayerCamera.h"
#include "IncludingJson.h"
#include "NetQueue.h"
#include <cassert>

Player::Player()
{
}

void Player::Update()
{
	Player* pPlayer = FindGameObject<Player>();
	assert(pPlayer == this);
	/* これ、移動に角度必要じゃね？ */
	//前移動
	if (CheckHitKey(KEY_INPUT_W))
	{
		playerState.position.z += SPEED;
		//playerState.position.z += playerState.rotate.y * SPEED;	みたいな奴を花井さんの授業でやったはず...。
	}
	//後移動
	if (CheckHitKey(KEY_INPUT_S))
	{
		playerState.position.z -= SPEED;
	}
	//左移動
	if (CheckHitKey(KEY_INPUT_A))
	{
		playerState.position.x -= SPEED;
	}
	//右移動
	if (CheckHitKey(KEY_INPUT_D))
	{
		playerState.position.x += SPEED;
	}

	NetQueue* pNetQueue = FindGameObject<NetQueue>();
	pNetQueue->Send(json
		{
			{ "head", "Update" },
			{
				"content",
				{
					"position",
					{
						{ "x", playerState.position.x },
						{ "y", playerState.position.y },
						{ "z", playerState.position.z },
					},
				},
			},
		}.dump(), UDP);
}

void Player::Draw()
{
}

Player::~Player()
{
}

void Player::PushPData(const json data)
{
	NetQueue* queue = FindGameObject<NetQueue>();
	queue->Send(data.dump(), TCP);
}

void Player::ChangeNameImGui()
{
	//char* newName = nullptr;
	char newName[20];
	ImGui::Begin("変更したいユーザー名を入力してください。");
	ImGui::InputText("new Name : ", newName, 20);
	ImGui::End();

	std::string name(newName);
	json nameJson =
	{
	  { "head", "Event" },
	  { "content",
		{
		  "head", "Name",
		  "content", name
		}
	  }
	};
	PushPData(nameJson);
	ChangeName(name);
}

void Player::ChangeColorImGui()
{
	float myColor[3] = {0,0,0};

	if (ImGui::ColorPicker3("色変更", myColor, ImGuiColorEditFlags_Uint8))
	{
		DxLib::COLOR_U8 color;
		color.r = myColor[Color::R];
		color.g = myColor[Color::G];
		color.b = myColor[Color::B];

		json colorJson =
		{
		  { "head", "Event" },
		  { "content",
			{
			  "head", "Color",
			  "content", GetColor(color.r,color.g,color.b)
			}
		  }
		};
		PushPData(colorJson);
		ChangeColor(color);
	}
}

void Player::TextChatImGui()
{
	//char* inputChat = nullptr;
	char inputChat[60];
	ImGui::Begin("テキストを入力...");
	ImGui::InputText("", inputChat, 60);
	ImGui::End();

	std::string chat(inputChat);
	json chatJson =
	{
	  { "head", "Event" },
	  { "content",
		{
		  "head", "Chat",
		  "content", chat
		}
	  }
	};
	PushPData(chatJson);
}

void Player::SetMyCamera()
{
	PlayerCamera* camera = FindGameObject<PlayerCamera>();
	camera->SetPosition(playerState.position);
	//playerState.rotate = camera->GetRotate();
	camera = nullptr;
}

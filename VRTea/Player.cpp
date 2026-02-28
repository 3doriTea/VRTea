#include "Player.h"
#include "PlayerCamera.h"
#include "IncludingJson.h"
#include "NetQueue.h"
#include <cassert>
#include <DirectXMath.h>


namespace
{
	static const float PLAYER_EYE_HEIGHT{ 1.6 };
}


Player::Player() :
	playerState{},
	pData{}
{
	NetQueue* pNetQueue = FindGameObject<NetQueue>();
	assert(pNetQueue && "NetQueueが見つからない");

	if (pNetQueue)
	{
		pNetQueue->Send(json
			{
				{ "head", "Join" },
			}.dump(), TCP);
	}
}

void Player::Update()
{
	Player* pPlayer = FindGameObject<Player>();
	assert(pPlayer == this);
	PlayerCamera* pPlayerCamera = FindGameObject<PlayerCamera>();
	assert(pPlayerCamera && "プレイヤーカメラが見つからんかった");


	float angleY = pPlayerCamera->GetAngleY();
	DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(angleY);
	

	/* これ、移動に角度必要じゃね？ */  // ← 必要です。
	DirectX::XMVECTOR localMove = {};
	enum
	{
		X, Y, Z, W
	};

	//前移動
	if (CheckHitKey(KEY_INPUT_W))
	{
		localMove.m128_f32[Z] += SPEED;
		//playerState.position.z += SPEED;
		//playerState.position.z += playerState.rotate.y * SPEED;	みたいな奴を花井さんの授業でやったはず...。
		// ↑ 回転行列です。
	}
	//後移動
	if (CheckHitKey(KEY_INPUT_S))
	{
		localMove.m128_f32[Z] -= SPEED;
		//playerState.position.z -= SPEED;
	}
	//左移動
	if (CheckHitKey(KEY_INPUT_A))
	{
		localMove.m128_f32[X] -= SPEED;
		//playerState.position.x -= SPEED;
	}
	//右移動
	if (CheckHitKey(KEY_INPUT_D))
	{
		localMove.m128_f32[X] += SPEED;
		//playerState.position.x += SPEED;
	}

	// ワールド上の移動を取得
	DirectX::XMVECTOR worldMove = DirectX::XMVector3TransformCoord(localMove, rot);
	// ワールド上の移動を加算
	playerState.position.x += worldMove.m128_f32[X];
	playerState.position.y += worldMove.m128_f32[Y];
	playerState.position.z += worldMove.m128_f32[Z];

	NetQueue* pNetQueue = FindGameObject<NetQueue>();
	assert(pNetQueue && "NetQueueが見つからない");
	if (pNetQueue)
	{
		pNetQueue->Send(json
			{
				{ "head", "Update" },
				{
					"content",
					json
					{
						"position",
						{
							{ "x", playerState.position.x },
							{ "y", playerState.position.y },
							{ "z", playerState.position.z },
						},
					},
				},
			}.dump(), TCP);

		pPlayerCamera->SetPosition(VAdd(playerState.position, VGet(0, PLAYER_EYE_HEIGHT, 0)));
	}
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

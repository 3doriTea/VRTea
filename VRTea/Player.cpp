#include "Player.h"
#include "PlayerCamera.h"
#include "IncludingJson.h"
#include "NetQueue.h"
#include <cassert>
#include <DirectXMath.h>


namespace
{
	static const float PLAYER_EYE_HEIGHT{ 1.6 };

	static GameMode STATE_GAME_MODE{};
	static MenuMode STATE_MENU_MODE{};
}


Player::Player() :
	playerState{},
	pData{},
	pPlayModeState_{ &STATE_GAME_MODE },
	prevMKeyPressed_{ false },
	nameBuffer{},
	colorBuffer{}
{
	ImGuiIO& io = ImGui::GetIO();

	// 1. フォント設定 (日本語を表示するための設定)
	ImFontConfig config;
	config.MergeMode = false; // 既存のフォントに上書きせず新しく作るならfalse

	// 2. 日本語の範囲を指定 (GetGlyphRangesJapanese() を使うのがミソ)
	const ImWchar* glyph_ranges = io.Fonts->GetGlyphRangesJapanese();

	// 3. フォントファイルをロード (Windows標準フォントの例)
	// サイズは 18.0f くらいが見やすいです
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 18.0f, &config, glyph_ranges);

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
	bool currMKeyPressed = CheckHitKey(KEY_INPUT_M);

	// ステートの切り替え
	if (prevMKeyPressed_ != currMKeyPressed && currMKeyPressed)
	{
		pPlayModeState_->Exit(*this);  // 変更だよ
		if (pPlayModeState_ != &STATE_GAME_MODE)
		{
			pPlayModeState_ = &STATE_GAME_MODE;
		}
		else
		{
			pPlayModeState_ = &STATE_MENU_MODE;
		}
	}
	prevMKeyPressed_ = currMKeyPressed;

	pPlayModeState_->Update(*this);
}

void GameMode::Update(Player& _p)
{
	Player* pPlayer = _p.FindGameObject<Player>();
	assert(pPlayer == &_p);
	PlayerCamera* pPlayerCamera = _p.FindGameObject<PlayerCamera>();
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
		localMove.m128_f32[Z] += _p.SPEED;
		//playerState.position.z += SPEED;
		//playerState.position.z += playerState.rotate.y * SPEED;	みたいな奴を花井さんの授業でやったはず...。
		// ↑ 回転行列です。
	}
	//後移動
	if (CheckHitKey(KEY_INPUT_S))
	{
		localMove.m128_f32[Z] -= _p.SPEED;
		//playerState.position.z -= SPEED;
	}
	//左移動
	if (CheckHitKey(KEY_INPUT_A))
	{
		localMove.m128_f32[X] -= _p.SPEED;
		//playerState.position.x -= SPEED;
	}
	//右移動
	if (CheckHitKey(KEY_INPUT_D))
	{
		localMove.m128_f32[X] += _p.SPEED;
		//playerState.position.x += SPEED;
	}

	// ワールド上の移動を取得
	DirectX::XMVECTOR worldMove = DirectX::XMVector3TransformCoord(localMove, rot);
	// ワールド上の移動を加算
	_p.playerState.position.x += worldMove.m128_f32[X];
	_p.playerState.position.y += worldMove.m128_f32[Y];
	_p.playerState.position.z += worldMove.m128_f32[Z];

	NetQueue* pNetQueue = _p.FindGameObject<NetQueue>();
	assert(pNetQueue && "NetQueueが見つからない");
	if (pNetQueue)
	{
		pNetQueue->Send(json
			{
				{ "head", "Update" },
				{ "content",
					{
						{ "position",
							{
								{ "x", _p.playerState.position.x },
								{ "y", _p.playerState.position.y },
								{ "z", _p.playerState.position.z }
							}
						}
					}
				}
			}.dump(), UDP);

		pPlayerCamera->SetPosition(VAdd(_p.playerState.position, VGet(0, PLAYER_EYE_HEIGHT, 0)));
	}
}

void MenuMode::Enter(Player& _p)
{
	memset(_p.nameBuffer, 0x00, NAME_BUFFER_SIZE);
	memcpy(_p.nameBuffer, _p.pData.name.c_str(), _p.pData.name.size());


	_p.colorBuffer[COLOR_AT_R] = _p.pData.color.r / 255.0f;
	_p.colorBuffer[COLOR_AT_G] = _p.pData.color.g / 255.0f;
	_p.colorBuffer[COLOR_AT_B] = _p.pData.color.b / 255.0f;
}

void MenuMode::Update(Player& _p)
{
	ImGui::Begin("Enter your new username.");
	ImGui::InputText("##new Name : ", _p.nameBuffer, NAME_SIZE_MAX);
	ImGui::End();

	if (ImGui::BeginPopup("MyPopup")) 
	{
		// 2列のテーブルを作成 (境界線あり: ImGuiTableFlags_BordersInnerV)
		if (ImGui::BeginTable("split_table", 2, ImGuiTableFlags_BordersInnerV))
		{

			// 左側の列
			ImGui::TableNextColumn();
			ImGui::Text("--- 左側 ---");
			ImGui::Button("設定A");
			ImGui::Button("設定B");

			// 右側の列
			ImGui::TableNextColumn();
			ImGui::Text("--- 右側 ---");
			static char buf[32] = "";
			ImGui::InputText("##Name", buf, 32);

			ImGui::EndTable();
		}
		ImGui::EndPopup();
	}

	if (ImGui::ColorPicker3("Pick your body color.", _p.colorBuffer, ImGuiColorEditFlags_Uint8))
	{
		COLOR_U8 color{};
		color.r = static_cast<BYTE>(_p.colorBuffer[Color::R] * 255.0f);
		color.g = static_cast<BYTE>(_p.colorBuffer[Color::G] * 255.0f);
		color.b = static_cast<BYTE>(_p.colorBuffer[Color::B] * 255.0f);

		json colorJson =
		{
			{ "head", "Event" },
			{ "content",
				{
					{ "head", "NewColor" },
					{ "content", GetColor(color.r, color.g, color.b) },
				}
			}
		};
		_p.PushPData(colorJson);
		_p.ChangeColor(color);
	}
}

void MenuMode::Exit(Player& _p)
{
	_p.pData.name = _p.nameBuffer;
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
	ImGui::InputText("Input", inputChat, 60);
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

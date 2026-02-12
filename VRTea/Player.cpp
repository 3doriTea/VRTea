#include "Player.h"
#include "PlayerCamera.h"
#include "IncludingJson.h"
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


}

void Player::Draw()
{
}

Player::~Player()
{
}

void Player::ChangeNameImGui()
{
	char* newName;
	ImGui::Begin("変更したいユーザー名を入力してください。");
	ImGui::InputText("new Name : ", newName, 20);
	ImGui::End();

	std::string name(newName);
	ChangeName(name);
}

void Player::ChangeColorImGui()
{
	DxLib::COLOR_U8 color;
	ImGui::Begin("色変更");
	ImGui::End();

	ChangeColor(color);
}

void Player::SetMyCamera()
{
	PlayerCamera* camera = FindGameObject<PlayerCamera>();
	camera->SetPosition(playerState.position);
	//playerState.rotate = camera->GetRotate();
	camera = nullptr;
}

#include "Player.h"
#include <cassert>


Player::Player()
{
}

void Player::Update()
{
	Player* pPlayer = FindGameObject<Player>();
	assert(pPlayer == this);
}

void Player::Draw()
{
}

Player::~Player()
{
}

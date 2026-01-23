#pragma once
#include "GameObject.h"

struct Player : GameObject
{
	Player();

	void Update() override;
	void Draw() override;

	~Player();

	void ChangeName(const std::string& _name);
	void ChangeColor(const int _color);
};

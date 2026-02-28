#pragma once
#include "GameObject.h"

struct OtherPlayerData
{
	inline OtherPlayerData(
		const std::string& _name,
		const VECTOR& _position,
		unsigned int _color) :
		name{ _name },
		position{ _position },
		color{ _color }
	{
	}

	std::string name;
	VECTOR position;
	unsigned int color;
};

struct OtherPlayer : GameObject
{
	OtherPlayer();
	~OtherPlayer();
	void Update() override;
	void Draw() override;

private:
	std::vector<OtherPlayerData> data_;
};
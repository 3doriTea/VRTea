#pragma once
#include <DxLib.h>
#include <string>
#include "IncludingJson.h"

struct PlayerData
{
	DxLib::COLOR_U8 color;
	std::string name;
};

struct GetPlayerData
{
	unsigned int color;
	std::string name;
};
enum Color
{
	R = 0,
	G,
	B
};

namespace ns {
	static void to_json(json& j, const PlayerData& p) {
		j = json{ {"name", p.name}, { "color", DxLib::GetColor(p.color.r, p.color.g, p.color.b) } };
	}

	static void from_json(const json& j, PlayerData& p_) {
		GetPlayerData p = {};
		const unsigned int RGB = 3;
		unsigned int getColor[RGB] = { 0,0,0 };
		const unsigned int char_BIT_SIZE = 8;
		j.at("name").get_to(p_.name);       // get_to(T& arg) は arg = get<T>() と同じ
		j.at("color").get_to(p.color);  // TODO: 色をまずunsigned int 型で受け取って、ビット操作で各色を分離させてください！
		for (int i = 0; i < RGB; i++)
		{
			for (int j = 1; j <= char_BIT_SIZE; j++)
			{
				unsigned int getBit = i * char_BIT_SIZE + j;
				getColor[i] += ((p.color >> getBit) & 1) * j;
			}
		}
		p_.color.r = getColor[R];
		p_.color.g = getColor[G];
		p_.color.b = getColor[B];
	}
}
#pragma once
#include <DxLib.h>
#include <string>
#include "IncludingJson.h"

struct PlayerData
{
	DxLib::COLOR_U8 color;
	std::string name;
};

namespace ns {
	static void to_json(json& j, const PlayerData& p) {
		j = json{ {"name", p.name}, { "color", DxLib::GetColor(p.color.r, p.color.g, p.color.b) } };
	}

	static void from_json(const json& j, PlayerData& p) {
		j.at("name").get_to(p.name);       // get_to(T& arg) は arg = get<T>() と同じ
		//j.at("color").get_to(p.color);  // TODO: 色をまずunsigned int 型で受け取って、ビット操作で各色を分離させてください！
	}
}
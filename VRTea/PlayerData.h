#pragma once
#include <DxLib.h>
#include <string>
struct PlayerData
{
	DxLib::COLOR_U8 color;
	std::string name;
};

namespace ns {
    void to_json(json& j, const PlayerData& p) {
        j = json{{"name", p.name}, {"color", p.color}};
    }

    void from_json(const json& j, PlayerData& p) {
        j.at("name").get_to(p.name);       // get_to(T& arg) ‚Í arg = get<T>() ‚Æ“¯‚¶
        j.at("color").get_to(p.color);
    }
}
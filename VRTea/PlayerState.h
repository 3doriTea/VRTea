#pragma once
#include "IncludingWin.h"
#include "IncludingJson.h"

struct PlayerState
{
	DxLib::VECTOR position;
};

namespace ns
{
	static void to_json(json& j, const PlayerState& p)
	{
		j["head"] = "Update";
		const DxLib::VECTOR& pos = p.position;
		json posJson;
		posJson["position"]["x"] = pos.x;
		posJson["position"]["y"] = pos.y;
		posJson["position"]["z"] = pos.z;
		j["content"] = posJson;
	}

	static void from_json(const json& j, PlayerState& s)
	{
		json posJson = j.at("content").at("position");
		s.position.x = posJson.at("x").get<float>();
		s.position.y = posJson.at("y").get<float>();
		s.position.z = posJson.at("z").get<float>();
	}
}
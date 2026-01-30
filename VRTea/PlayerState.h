#pragma once
#include <DxLib.h>
#include "IncludingJson.h"

struct PlayerState
{
	DxLib::VECTOR position;
};

namespace ns
{
	static void to_json(json& j, const PlayerState& p)
	{
		j["Head"] = "Update";
		const DxLib::VECTOR& pos = p.position;
		json posJson;
		posJson["position"]["x"] = pos.x;
		posJson["position"]["y"] = pos.y;
		posJson["position"]["z"] = pos.z;
		j["Content"] = posJson;
	}

	static void from_json(const json& j, PlayerState& s)
	{
		json posJson = j.at("Content").at("position");
		s.position.x = posJson.at("x").get<float>();
		s.position.y = posJson.at("y").get<float>();
		s.position.z = posJson.at("z").get<float>();
	}
}
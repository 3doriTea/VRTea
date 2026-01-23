#pragma once
#include <nlohmann/json.hpp>
struct Serializable
{
	Serializable() = default;
	virtual ~Serializable() = default;

	virtual bool TryToJson(nlohmann::json& _writeTo) { return false; }
	virtual bool TryFromJson(const nlohmann::json& _readTo) { return false; }
};

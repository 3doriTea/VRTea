#pragma once
#include <string_view>


namespace Logger
{
	void WriteOut(const std::string_view _content, const std::string_view _name = "");
}

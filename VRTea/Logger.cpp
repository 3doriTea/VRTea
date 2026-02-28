#define _CRT_SECURE_NO_WARNINGS

#include "Logger.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cassert>



void Logger::WriteOut(const std::string_view _content, const std::string_view _name)
{
	auto now = std::chrono::system_clock::now();
	std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
	std::tm* nowTm = std::localtime(&nowTime);

	std::stringstream fileName{};

	fileName << "_log" << _name << std::put_time(nowTm, "%H%M%S") << ".txt";

	std::ofstream ofs(fileName.str());
	assert(ofs && "ログファイル作成に失敗");

	ofs << _content;

	ofs.close();
}

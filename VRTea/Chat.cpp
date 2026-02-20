#include "Chat.h"
#include <nlohmann/json.hpp>
#include <format>
#include "../ImGui/imgui.h"
#include "NetQueue.h"
#include "NetQueueStub.h"
Chat::Chat()
	: showLogWindow_{true}
{
}
void Chat::Update()
{
	ReadContent();

	if (ImGui::IsKeyPressed(ImGuiKey_Tab))
	{
		showLogWindow_ = !showLogWindow_;
	}
}

void Chat::Draw()
{
	if (showLogWindow_)
	{
		DrawChatLog();
	}
}

void Chat::DrawChatLog()
{
	ImGui::Begin("Log", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	for (const std::string& text : chatLog_)
	{
		ImGui::Text("%s", text.c_str());
		ImGui::Separator();
	}
	
	// 既に一番下にスクロールしている場合
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
	{
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::End();
}

void Chat::DrawInputField()
{

}

void Chat::InputText()
{
	
}

void Chat::ReadContent()
{
	using namespace nlohmann;

	NetQueueStub* netQueue = FindGameObject<NetQueueStub>();
	if (netQueue == nullptr)
		return;
	std::string str = netQueue->Read();
	while (str != "")
	{
		json j = json::parse(str);
		// Headの存在をチェック
		if (j.contains("Head") == false)
			return;

		std::string head = j.at("Head").get<std::string>();
		std::string content = j.at("Content").get<std::string>();

		std::string log = std::format("Event:{} Content:{}", head, content);
		chatLog_.push_back(log);

		str = netQueue->Read();
	}
}

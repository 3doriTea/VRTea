#include "Chat.h"
#include <nlohmann/json.hpp>
#include <format>
#include "../ImGui/imgui.h"
#include "NetQueue.h"
#include "NetQueueStub.h"
Chat::Chat()
	: showLogWindow{true}
{
}
void Chat::Update()
{
	ReadContent();

	if (ImGui::IsKeyPressed(ImGuiKey_Tab))
	{
		showLogWindow = !showLogWindow;
	}
}

void Chat::Draw()
{
	if (showLogWindow)
	{
		DrawChatLog();
	}
}

void Chat::DrawChatLog()
{
	ImGui::Begin("Log", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	for (const ChatContent& chatContent : chatLog)
	{
		std::string_view sender = chatContent.sender;
		std::string_view message = chatContent.message;
		std::string text = std::format("Sender : {}\nMessage : {}", sender, message);
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

	
	json chatArrJson = netQueue->Find("Chat");
	// jsonが配列と決めつけている
	if (chatArrJson.is_array() == false)
		return;

	for (auto itr = chatArrJson.begin(); itr != chatArrJson.end(); itr++)
	{
		json chatJson = *itr;

		json content = chatJson.at("content");
		std::string message = content.at("message");
		std::string sender = content.at("sender");
		ChatContent chatContent
		{
			.sender = sender,
			.message = message
		};

		chatLog.push_back(chatContent);
	}
}

std::string Chat::GetSenderMessage(std::string_view sender)
{
	// 逆順で検索して、最新のを取得
	for (auto ritr = chatLog.rbegin(); ritr != chatLog.rend(); ritr++)
	{
		if (ritr->sender == sender)
			return ritr->message;
	}
	return "";
}

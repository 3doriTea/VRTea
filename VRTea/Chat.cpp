#include "Chat.h"
#include <nlohmann/json.hpp>
#include <format>
#include "../ImGui/imgui.h"
#include "NetQueue.h"
#include "NetQueueStub.h"
#include "Logger.h"
#include <algorithm>

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

	//printfDx("%d", chatLog.size());
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

	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(500, 200), 0);
	ImGui::Begin("Log", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	for (const ChatContent& chatContent : chatLog)
	{
		std::string_view sender = chatContent.sender;
		std::string_view message = chatContent.message;
		std::string text = std::format("[{}]{}", sender, message);
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


void Chat::ReadContent()
{
	using namespace nlohmann;

	NetQueue* netQueue = FindGameObject<NetQueue>();
	if (netQueue == nullptr)
	{
		assert(netQueue && "NetQueueが見つからない！");
		return;
	}

	json eventContentJson = netQueue->Find("Event");
	if (eventContentJson.empty())
	{
		return;
	}

	Logger::WriteOut(eventContentJson.dump(), "inContent");

	// Event の種類がまたある
	std::string eventHead = eventContentJson.value("head", "undefined");
	
	if (eventHead == "Chat")
	{
		const json& content = eventContentJson.at("content");
		std::string message = content.at("message");
		std::string sender = content.at("sender");
		int32_t senderId = content.value("senderId", -1);
		ChatContent chatContent
		{
			.sender = sender,
			.message = message,
			.senderId = senderId
		};

		chatLog.push_back(chatContent);

		for (auto& handler : chatEventHandlers)
		{
			handler(chatContent);
		}
	}
	else
	{
		assert(false && "未対応のイベントタイプ");
		return;
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

int Chat::RegisterChatEventHandler(std::function<void(const ChatContent&)> handler)
{
	chatEventHandlers.push_back(handler);
	return chatEventHandlers.size() - 1;
}

void Chat::UnregisterChatEventHandler(int handle)
{
	if (chatEventHandlers.size() <= handle || handle < 0)
		return;

	chatEventHandlers.erase(chatEventHandlers.begin() + handle);
}

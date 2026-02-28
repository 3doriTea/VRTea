#pragma once
#include "GameObject.h"
#include <vector>
#include <string>

struct ChatContent
{
	std::string sender;
	std::string message;
};

struct Chat : public GameObject
{
	Chat();
	void Update() override;
	void Draw() override;
	/// <summary>
	/// チャットのログを表示する
	/// </summary>
	void DrawChatLog();
	void DrawInputField();
	void InputText();
	/// <summary>
	/// NetQueueからチャット内容を読み取る
	/// </summary>
	void ReadContent();
	/// <summary>
	/// チャット送信者のメッセージを返す
	/// 最新のメッセージを返す
	/// </summary>
	/// <param name="sender">チャット送信者の名前</param>
	/// <returns></returns>
	std::string GetSenderMessage(std::string_view sender);
private:
	// チャットのログを保存するコンテナ
	std::vector<ChatContent> chatLog;
	// チャットのログ開閉フラグ
	bool showLogWindow;
};


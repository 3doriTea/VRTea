#pragma once
#include "GameObject.h"
#include <vector>
#include <string>
#include <functional>
#include "IncludingJson.h"
struct ChatContent
{
	std::string sender;
	std::string message;
	int32_t senderId;
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
	/// <summary>
	/// チャットイベントに対するハンドラーを登録する
	/// </summary>
	/// <param name="handler"></param>
	/// <returns>ハンドラーの識別子。UnregisterChatEventHandlerに渡して、解除に使う</returns>
	int RegisterChatEventHandler(std::function<void(const ChatContent&)> handler);
	/// <summary>
	/// ハンドラーを登録解除する
	/// </summary>
	/// <param name="handle">RegisterChatEventHandlerに返されたハンドラーの識別子</param>
	void UnregisterChatEventHandler(int handle);
private:
	// チャットのログを保存するコンテナ
	std::vector<ChatContent> chatLog;
	// チャットのログ開閉フラグ
	bool showLogWindow;
	std::vector<std::function<void(const ChatContent&)>> chatEventHandlers;
};


#pragma once
#include "GameObject.h"
#include <vector>
#include <string>
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
private:
	// チャットのログを保存するコンテナ
	std::vector<std::string> chatLog_;
	// チャットのログ開閉フラグ
	bool showLogWindow_;
};
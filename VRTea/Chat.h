#pragma once
#include "GameObject.h"

struct Chat : public GameObject
{
	void Update() override;
	void Draw() override;
	void DrawChatLog();
	void DrawInputField();
	void InputText();
};
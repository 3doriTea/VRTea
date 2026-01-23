#include <DxLib.h>
#include "GameObject.h"
#include "GameObjectWorld.h"

#include "Player.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1)
	{
		return -1;
	}

	GameObjectWorld world = {};

#pragma region ゲームオブジェクトはここでインスタンス作ってください

	GameObject::Instantiate<Player>();

#pragma 

	while (CheckHitKeyAll() == 0)
	{
		world.Update();
		world.Draw();

		if (ProcessMessage() == -1)
		{
			break;
		}
	}

	DxLib_End();

	return 0;
}

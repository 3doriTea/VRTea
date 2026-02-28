#include "GameObject.h"
#include <DxLib.h>
#include "GameObjectWorld.h"

#include "Player.h"
#include "PlayerCamera.h"
#include "OtherPlayer.h"
#include "NetQueue.h"
#include "Chat.h"
#include "GameTime.h"
#include "../ImGui/imgui_impl_dxlib.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetOutApplicationLogValidFlag(FALSE);
	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(TRUE);
	SetWindowSizeExtendRate(1.0);
	if (DxLib_Init() == -1)
	{
		return -1;
	}
	SetHookWinProc([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)-> LRESULT /*CALLBACK*/
		{
			// DxLibとImGuiのウィンドウプロシージャを両立させる
			SetUseHookWinProcReturnValue(FALSE);
			return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		});

	SetDrawScreen(DX_SCREEN_BACK);
	SetAlwaysRunFlag(TRUE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	/*io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;*/
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\meiryo.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());	ImGui_ImplDXlib_Init();

	GameObjectWorld world = {};

#pragma region ゲームオブジェクトはここでインスタンス作ってください

	GameObject::Instantiate<NetQueue>();
	GameObject::Instantiate<Player>();
	GameObject::Instantiate<PlayerCamera>();
	GameObject::Instantiate<OtherPlayer>();
	GameObject::Instantiate<Chat>();

#pragma 

	GameTime::Initialize();

	while (true)
	{
		ImGui_ImplDXlib_NewFrame();
		ImGui::NewFrame();

		GameTime::Refresh();

		world.Update();
		world.Draw();
		if (ProcessMessage() == -1)
		{
			break;
		}
		ScreenFlip();
		ClearDrawScreen();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDXlib_RenderDrawData();

		RefreshDxLibDirect3DSetting();
		//if (ImGui::GetIO().ConfigFlags )
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	ImGui_ImplDXlib_Shutdown();
	ImGui::DestroyContext();

	DxLib_End();

	return 0;
}

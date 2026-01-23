#include <DxLib.h>
#include "../ImGui/imgui_impl_dxlib.hpp"
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

	GameObject::Instantiate<Player>();

#pragma 

	while (true)
	{
		ImGui_ImplDXlib_NewFrame();
		ImGui::NewFrame();

		world.Update();
		world.Draw();
		ImGui::Begin("Hoge");
		ImGui::Text("ssh");
		ImGui::End();
		if (ProcessMessage() == -1)
		{
			break;
		}
		ScreenFlip();
		ClearDrawScreen();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDXlib_RenderDrawData();

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

#include "GameObject.h"
#include <DxLib.h>
#include "GameObjectWorld.h"

#include "Player.h"
#include "PlayerCamera.h"
#include "OtherPlayer.h"
#include "NetQueue.h"
#include "Chat.h"
#include "ChatWorld.h"
#include "GameTime.h"
#include "../ImGui/imgui_impl_dxlib.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetDoubleStartValidFlag(TRUE);

	SetOutApplicationLogValidFlag(FALSE);
	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(TRUE);
	SetWindowSizeExtendRate(1.0);
	SetUseIMEFlag(TRUE);
	SetUseTSFFlag(FALSE);
	if (DxLib_Init() == -1)
	{
		return -1;
	}
	SetHookWinProc([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)-> LRESULT /*CALLBACK*/
		{
			if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			{
				// DxLibとImGuiのウィンドウプロシージャを両立させる
				SetUseHookWinProcReturnValue(TRUE);
				return true;
			}
			switch (msg)
			{
			case WM_IME_SETCONTEXT:
			case WM_IME_STARTCOMPOSITION:
			case WM_IME_ENDCOMPOSITION:
			case WM_IME_COMPOSITION:
			case WM_IME_NOTIFY:
			case WM_IME_REQUEST:
				SetUseHookWinProcReturnValue(TRUE);
				return DefWindowProc(hWnd, msg, wParam, lParam);

			case WM_SYSCOMMAND:
				if ((wParam & 0xfff0) == SC_KEYMENU) { // Disable ALT application menu
					SetUseHookWinProcReturnValue(TRUE);
					return 0;
				}
				break;

			}
			return 0;
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
	GameObject::Instantiate<PlayerCamera>();
	GameObject::Instantiate<OtherPlayer>();
	GameObject::Instantiate<ChatWorld>();
	GameObject::Instantiate<Player>();
	GameObject::Instantiate<Chat>();
	OtherPlayer* pOtherPlayer = GameObject::FindGameObject<OtherPlayer>();
	pOtherPlayer->SetChatEventHandler();


#pragma 

	GameTime::Initialize();

	while (true)
	{
		if (ProcessMessage() == -1)
		{
			break;
		}

		ImGui_ImplDXlib_NewFrame();
		ImGui::NewFrame();

		GameTime::Refresh();

		world.Update();
		world.Draw();
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDXlib_RenderDrawData();

		ScreenFlip();
		ClearDrawScreen();


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

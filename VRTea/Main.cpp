#include <DxLib.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (DxLib_Init() == -1)
	{
		return -1;
	}

	while (CheckHitKeyAll() == 0)
	{
		if (ProcessMessage() == -1)
		{
			break;
		}
	}

	DxLib_End();

	return 0;
}

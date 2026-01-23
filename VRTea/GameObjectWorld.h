#pragma once


/// <summary>
/// ゲームオブジェクトの更新と描画をするためだけの世界
/// </summary>
class GameObjectWorld final
{
public:
	GameObjectWorld();
	~GameObjectWorld();

	void Update();
	void Draw();
};

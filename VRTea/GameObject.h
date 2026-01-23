#pragma once
#include <list>
#include <typeindex>
#include <map>
#include <string>
#include <string_view>
#include "IncludingJson.h"


/// <summary>
/// <para>ゲームオブジェクト</para>
/// <para>継承して使ってね！</para>
/// </summary>
class GameObject
{
	friend class GameObjectWorld;
protected:
	GameObject();
	virtual ~GameObject() {}

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() {}
	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw() {}

/* ----- STATIC ----- */
public:
	/// <summary>
	/// ゲームオブジェクトを生成する
	/// </summary>
	/// <typeparam name="GameObjectT">生成するゲームオブジェクトの型</typeparam>
	/// <typeparam name="...Args">コンストラクタに渡す引数</typeparam>
	/// <param name="..._args">コンストラクタに渡す可変長引数</param>
	template<typename GameObjectT, typename ...Args>
	static void Instantiate(Args ..._args)
	{
		gameObjectCollection_.emplace(typeid(GameObjectT), nullptr);

		gameObjectCollection_.at(typeid(GameObjectT)) = new GameObjectT{ _args... };
	}

	/// <summary>
	/// 全ゲームオブジェクトを消し去る
	/// </summary>
	static void DeleteAll()
	{
		auto itr = gameObjectCollection_.begin();
		while (itr != gameObjectCollection_.end())
		{
			auto& [idx, pGameObject] = *itr;
			delete pGameObject;

			itr = gameObjectCollection_.erase(itr);
		}
	}

	/// <summary>
	/// ゲームオブジェクトを見つける
	/// </summary>
	/// <typeparam name="GameObjectT">ゲームオブジェクトの型</typeparam>
	/// <returns>
	/// <para>見つかったゲームオブジェクトのポインタ</para>
	/// <para>見つかったら nullptr</para>
	/// </returns>
	template<typename GameObjectT>
	static GameObjectT* FindGameObject()
	{
		if (gameObjectCollection_.contains(typeid(GameObjectT)))
		{
			return dynamic_cast<GameObjectT*>(
				gameObjectCollection_.at(
					typeid(GameObjectT)
				));
		}
		else
		{
			return nullptr;  // 見つからなかった！
		}
	}

private:
	static std::map<std::type_index, GameObject*> gameObjectCollection_;
};

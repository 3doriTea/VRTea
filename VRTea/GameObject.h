#pragma once
#include <list>
#include <typeindex>
#include <map>
#include <string>
#include <string_view>


class GameObject
{
	friend class GameObjectUpdater;
protected:
	GameObject();
	virtual ~GameObject() {}

	virtual void Update() {}
	virtual void Draw() {}

/* ----- STATIC ----- */
public:
	template<typename GameObjectT, typename ...Args>
	static void Instantiate(Args ..._args)
	{
		gameObjectCollection_.emplace(typeid(GameObjectT), new GameObjectT(_args...));
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
			return gameObjectCollection_.at(typeid(GameObjectT));
		}
		else
		{
			return nullptr;  // 見つからなかった！
		}
	}

private:
	static std::map<std::type_index, GameObject*> gameObjectCollection_;
};

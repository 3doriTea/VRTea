#include <iostream>
#include <list>

#pragma region 試作ゲームオブジェクト01 friend class で我慢する
#if 0

class GameObject
{
public:
	GameObject() {}

protected:
	virtual ~GameObject() {}

	virtual void Update() {}
	virtual void Draw() {}

public:
	template<typename GameObjectT>
	static GameObjectT* Instantiate()
	{
		GameObjectT* pGameObject{ new GameObjectT };
		gameObjects_.push_back(pGameObject);

		return pGameObject;
	}

private:
	static std::list<GameObject*> gameObjects_;
};

std::list<GameObject*> GameObject::gameObjects_{};


class Player : GameObject
{
	friend class GameObject;

	Player() {}
	~Player() {}

	void Update() override {}
	void Draw() override {}
};

#endif
#pragma endregion

#pragma region 試作ゲームオブジェクト02 struct にしちゃえ
#if 1

class GameObject
{
protected:
	GameObject() {}
	virtual ~GameObject() {}

	virtual void Update() {}
	virtual void Draw() {}

public:
	template<typename GameObjectT>
	static GameObjectT* Instantiate()
	{
		GameObjectT* pGameObject{ new GameObjectT };
		gameObjects_.push_back(pGameObject);

		return pGameObject;
	}

private:
	static std::list<GameObject*> gameObjects_;
};

std::list<GameObject*> GameObject::gameObjects_{};


struct Player : GameObject
{
	Player() {}
	~Player() {}

	void Update() override {}
	void Draw() override {}
};

#endif
#pragma endregion

int main()
{
	// プレイヤーを作る
	Player* player{ GameObject::Instantiate<Player>() };

	return 0;
}

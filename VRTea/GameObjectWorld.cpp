#include "GameObjectWorld.h"
#include "GameObject.h"

GameObjectWorld::GameObjectWorld()
{
}

GameObjectWorld::~GameObjectWorld()
{
}

void GameObjectWorld::Update()
{
	for (auto& [idx, pGameObject] : GameObject::gameObjectCollection_)
	{
		pGameObject->Update();
	}
}

void GameObjectWorld::Draw()
{
	for (auto& [idx, pGameObject] : GameObject::gameObjectCollection_)
	{
		pGameObject->Draw();
	}
}

#include "GameObjectUpdater.h"
#include "GameObject.h"

GameObjectUpdater::GameObjectUpdater()
{
}

GameObjectUpdater::~GameObjectUpdater()
{
}

void GameObjectUpdater::Update()
{
	for (auto& [idx, pGameObject] : GameObject::gameObjectCollection_)
	{
		pGameObject->Update();
	}
}

void GameObjectUpdater::Draw()
{
	for (auto& [idx, pGameObject] : GameObject::gameObjectCollection_)
	{
		pGameObject->Draw();
	}
}

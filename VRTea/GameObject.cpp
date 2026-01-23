#include "GameObject.h"

std::map<std::type_index, GameObject*> GameObject::gameObjectCollection_{};

GameObject::GameObject()
{
}

#include "GameObject.h"
#include <cassert>

std::map<std::type_index, GameObject*> GameObject::gameObjectCollection_{};

GameObject::GameObject()
{
}

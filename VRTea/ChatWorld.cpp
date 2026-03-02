#include "ChatWorld.h"

ChatWorld::ChatWorld()
{
}

ChatWorld::~ChatWorld()
{
}

void ChatWorld::Update()
{
}

void ChatWorld::Draw()
{
	VECTOR ChatWorldSize = VGet(500, -10, 500);
	DrawTriangle3D(ChatWorldSize, VGet(ChatWorldSize.x, ChatWorldSize.y, -ChatWorldSize.z), VGet(-ChatWorldSize.x, ChatWorldSize.y, -ChatWorldSize.z), GetColor(0, 128, 0), TRUE);
	DrawTriangle3D(ChatWorldSize, VGet(-ChatWorldSize.x, ChatWorldSize.y, ChatWorldSize.z), VGet(-ChatWorldSize.x, ChatWorldSize.y, -ChatWorldSize.z), GetColor(0, 128, 0), TRUE);
}

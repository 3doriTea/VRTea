#include "OtherPlayer.h"
#include "NetQueue.h"
#include "Chat.h"
#include "Player.h"
#include "GameTime.h"

/// <summary>
/// JSONから色情報を取得
/// </summary>
/// <param name="colorJson"></param>
/// <returns></returns>
DxLib::COLOR_U8 GetColorFromJson(const json& colorJson)
{
	// 色情報を取得
	unsigned int color = colorJson.get<unsigned int>();
	// 先頭を指すポインタ
	unsigned char* ptr = (unsigned char*)&color;
	// rgbを一時的に格納する配列
	unsigned char rgb[3];

	// r,g,bを抽出
	for (int i = 0; i < 3; i++)
	{
		rgb[2 - i] = *(ptr + sizeof(unsigned char) * i);
	}

	// COLOR_U8に入れて返す
	DxLib::COLOR_U8 resultColor;
	resultColor.r = rgb[0];
	resultColor.g = rgb[1];
	resultColor.b = rgb[2];
	return resultColor;
}


OtherPlayer::OtherPlayer()
	: otherPlayerCapsuleHeight_{10.0f}
	, otherPlayerCapsuleRadius_{5.0f}
	, otherPlayerCapsuleDivNum_{5}
	, otherPlayerChatDisplayTime_{5.0f}
	, hCallback{-1}
{
	

	
}

OtherPlayer::~OtherPlayer()
{
	Chat* pChat = FindGameObject<Chat>();
	if (pChat != nullptr)
	{
		pChat->UnregisterChatEventHandler(hCallback);
	}
}

void OtherPlayer::Update()
{
	NetQueue* pNetQueue = FindGameObject<NetQueue>();
	assert(pNetQueue && "NetQueueが見つからない");
	if (pNetQueue)
	{
		while (true)
		{
			json contentJ = pNetQueue->Find("Updated");
			if (contentJ.empty())
			{
				break;  // updatedが未受信ならループ終了
			}

			otherPlayersData_.clear();
			for (auto [id, data] : contentJ.items())
			{
				unsigned int color = data.value("color", 0xffffffffU);
				const json& positionJ = data.at("position");
				VECTOR posV = {};
				posV.x = positionJ.value("x", 0.0f);
				posV.y = positionJ.value("y", 0.0f);
				posV.z = positionJ.value("z", 0.0f);
				std::string name = data.value("name", "*");
				int32_t idInt = std::stoi(id);
				otherPlayersData_.push_back({ name, posV, color,idInt });
			}
		}

	}

	for (auto& [sender, chat] : otherPlayerChatMap_)
	{
		if (chat.timeLeft > 0.0f)
		{
			chat.timeLeft -= GameTime::DeltaTime();
		}
	}
}

void OtherPlayer::Draw()
{	
	//デバッグ確認
	DrawLine3D(VGet(-100.0f, 0.0f, 0.0f), VGet(100.0f, 0.0f, 0.0f), GetColor(255, 255, 255));
	DrawLine3D(VGet(-100.0f, 100.0f, 0.0f), VGet(100.0f, -100.0f, 0.0f), GetColor(255, 255, 255));

	DrawOtherPlayer();
}

void OtherPlayer::DrawOtherPlayer()
{
	for (const auto& data : otherPlayersData_)
	{
		// カラーコード取得
		//const DxLib::COLOR_U8& color = data.color;
		unsigned int colorCode = data.color;// DxLib::GetColor(color.r, color.g, color.b);

		// 座標取得
		const DxLib::VECTOR& position = data.position;

		// カプセル描画
		DrawCapsule3D(position, VGet(position.x, position.y + otherPlayerCapsuleHeight_, position.z), otherPlayerCapsuleRadius_, otherPlayerCapsuleDivNum_, colorCode, colorCode, TRUE);
		
		// メッセージボックス描画
		auto itr = otherPlayerChatMap_.find(data.id);
		if (itr == otherPlayerChatMap_.end())
			continue;
		if (itr->second.timeLeft <= 0.0f)
			continue;
		DrawMessageBox(position,data.name);
	}
}

void OtherPlayer::DrawMessageBox(const DxLib::VECTOR& playerPos, const std::string& sender)
{
	int boxSizeX = 128	, boxSizeY = 128;//テキストボックスのサイズ
	float capsuleHeight = 10.0f;//カプセルの高さ
	float capsuleWidth = 2.0f;//カプセルの幅
	float triangleHeight = capsuleHeight + capsuleWidth;//頂点の位置
	float textBoxHeight = triangleHeight + 4.0f;//テキストボックスの高さ調整
	VECTOR textBoxPos = ConvWorldPosToScreenPos(VGet(playerPos.x, playerPos.y + textBoxHeight, playerPos.z));
	VECTOR trianglePos = ConvWorldPosToScreenPos(VGet(playerPos.x, playerPos.y + triangleHeight, playerPos.z));	

	int triangleSizeX = 10.0f;//幅
	int triangleSizeY = textBoxPos.y;
	
	if (textBoxPos.z <= 1.0f)
	{
		//テキストボックスの枠
		DrawBox((textBoxPos.x - boxSizeX) - 3, (textBoxPos.y - boxSizeY) - 3, (textBoxPos.x + boxSizeX) + 3, textBoxPos.y + 3, GetColor(0, 0, 255), TRUE);
		DrawTriangle(trianglePos.x, (trianglePos.y) + 3, (trianglePos.x - triangleSizeX) - 3, triangleSizeY, (trianglePos.x + triangleSizeX) + 3, triangleSizeY, GetColor(0, 0, 255), TRUE);
		//テキストボックス
		DrawBox(textBoxPos.x - boxSizeX, textBoxPos.y - boxSizeY, textBoxPos.x + boxSizeX, textBoxPos.y, GetColor(255, 255, 255), TRUE);
		DrawTriangle(trianglePos.x, trianglePos.y, (trianglePos.x - triangleSizeX), triangleSizeY, (trianglePos.x + triangleSizeX), triangleSizeY, GetColor(255, 255, 255), TRUE);
	}

	Chat* chat = FindGameObject<Chat>();
	assert(chat && "Chatクラスが見つからない");
	// 送信者のメッセージを取得
	std::string senderMessage = chat->GetSenderMessage(sender);
	
	if (senderMessage != "")
	{
		int width = DxLib::GetDrawStringWidth(senderMessage.c_str(), senderMessage.size());
		int height = (textBoxPos.y - boxSizeY + textBoxPos.y)  / 2;

		DxLib::DrawStringF((textBoxPos.x - boxSizeX) + width, height, senderMessage.c_str(), GetColor(0, 0, 0), GetColor(255, 255, 255));
	}	

}

void OtherPlayer::SetChatEventHandler()
{
	Chat* chat = FindGameObject<Chat>();
	assert(chat && "Chatが見つからない");
	hCallback = chat->RegisterChatEventHandler(
		[this](const ChatContent& _content)
		{
			OtherPlayerChat otherPlayerChat =
			{
				.timeLeft = otherPlayerChatDisplayTime_,
				.content = _content.message
			};
			otherPlayerChatMap_[_content.senderId] = otherPlayerChat;
		});
	
}

#pragma once
#define WIN32_LEAN_AND_MEAN
#include "DxLib.h"
#include "GameObject.h"
#include <queue>
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <list>

#pragma comment(lib, "ws2_32.lib")

enum TCP_OR_UDP
{
	TCP, UDP
};

// RecvList用
struct Recv
{
	std::string head;
	json		body;
};

struct NetQueue : GameObject
{
NetQueue();				// コンストラクタ
~NetQueue();			// デストラクタ
void Update() override; // 更新
void Draw();			// 描画

// 送信したいデータをキューに積む（実送信はUpdate内）
void Send(const std::string& content, TCP_OR_UDP);

// 受信済みデータを取り出す（空ならfalse）
std::string Read(std::string& out);

// キューの中から発見
json Find(std::string TagName);

// ノンブロッキング化
void SetNonBlocking(SOCKET S);

// 接続処理
bool Connect(const char* ip, uint16_t port);

std::queue<std::string> sendQueue; // 送信待ち
std::queue<std::string> readQueue; // 受信済み

// ソケット作成
SOCKET sock = INVALID_SOCKET;

// UDP作成
SOCKET sockUDP;

// TCP作成
SOCKET sockTCP;

// TCP:受信したデータを一度貯める
std::string recvBuffer;

// 送信に備えて、送信中の未送信バッファを保持する
std::string sendingBuffer;

std::queue<std::string> sendQueueUDP;
std::queue<std::string> sendQueueTCP;

std::list<Recv>			RecvList;

bool connected = false;	// 接続用
};
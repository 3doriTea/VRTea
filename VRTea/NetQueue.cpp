#include "NetQueue.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <assert.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include "Logger.h"

using nlohmann::json;


namespace
{
	static const size_t BUFFER_SIZE{ 1024 };

	static const char* SERVER_IP_ADDRESS{ "192.168.33.2" };
	static const USHORT SERVER_PORT_NUMBER{ 3333U };
}


static bool ConnectImplTCP(const SOCKADDR_IN& addr, SOCKET s, int timeoutSec = 10)
{
	// 非ブロッキング化
	u_long mode = 1;
	ioctlsocket(s, FIONBIO, &mode);

	int ret = ::connect(s, (const SOCKADDR*)&addr, sizeof(addr));
	if (ret == 0)
	{
		// 直ちに接続完了
		return true;
	}
	if (ret == SOCKET_ERROR)
	{
		int wsa = WSAGetLastError();
		if (wsa != WSAEWOULDBLOCK && wsa != WSAEINPROGRESS)
		{
			return false;
		}
	}

	// 接続完了待ち: write-set と except-set を見る
	fd_set wfds, efds;
	FD_ZERO(&wfds); FD_SET(s, &wfds);
	FD_ZERO(&efds); FD_SET(s, &efds);

	timeval tv{};
	tv.tv_sec = timeoutSec;
	tv.tv_usec = 0;

	int sel = ::select(0, nullptr, &wfds, &efds, &tv);
	if (sel <= 0) return false; // タイムアウト or エラー

	// エラー発生は except-set に乗る。また SO_ERROR で最終確認。
	int soerr = 0;
	int len = sizeof(soerr);
	::getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&soerr, &len);

	if (FD_ISSET(s, &efds) || soerr != 0)
	{
		return false;
	}
	return FD_ISSET(s, &wfds) != 0;
}

static bool ConnectImplUDP(const SOCKADDR_IN& addr, SOCKET s)
{
	// UDP は connect() で既定の宛先を設定するだけ
	// 非ブロッキングにしておくのは送受信の都合上OK
	u_long mode = 1;
	ioctlsocket(s, FIONBIO, &mode);

	int ret = ::connect(s, (const SOCKADDR*)&addr, sizeof(addr));
	return (ret == 0);
}


// ノンブロッキング設定(そのまま次の処理に行く用)
void NetQueue::SetNonBlocking(SOCKET S)
{
	u_long mode = 1; 
	ioctlsocket(S, FIONBIO, &mode);
}

// コンストラクタ(ここは前回のネットワークの物を引用)
NetQueue::NetQueue() 
	: sockUdp(INVALID_SOCKET), sockTcp(INVALID_SOCKET) // 初期化の警告が出るので修正
{
	// WinSock2.2 初期化
	WSADATA wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
	{
		std::cout << "Error : WSAStartup" << std::endl;
		return;
	}
	std::cout << "Success : WSAStartup" << std::endl;

	// TCP ソケット作る
	sockTcp = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	assert(sockTcp != INVALID_SOCKET && "Error : listenSock");
	
	unsigned long arg = 0x01;
	int result = ioctlsocket(sockTcp, FIONBIO, &arg);
	assert(result != SOCKET_ERROR);

	int ret{};

	// TCP ソケットをバインドする
	sockaddr_in localTcpAddr{};
	localTcpAddr.sin_family = AF_INET;
	localTcpAddr.sin_port = htons(0);  // 適切なポート番号を探してくれ―――
	localTcpAddr.sin_addr.s_addr = INADDR_ANY;  // どこからでもアクセスokだよ

	ret = bind(sockTcp, reinterpret_cast<sockaddr*>(&localTcpAddr), sizeof(localTcpAddr));
	assert(ret != SOCKET_ERROR);
	
	// TCP ソケットのポート番号を取得する
	sockaddr_in bindedLocalTcpAddr{};
	int addrLength = sizeof(bindedLocalTcpAddr);
	ret = getsockname(sockTcp, reinterpret_cast<sockaddr*>(&bindedLocalTcpAddr), &addrLength);
	assert(ret == 0);

	// TCPとUDP共有のポート番号
	USHORT commonPort = ntohs(bindedLocalTcpAddr.sin_port);

	// UDP ソケットつくる
	sockUdp = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(sockUdp != INVALID_SOCKET && "Error : socketUDP");

	// UDP ソケットをバインドする
	sockaddr_in localUDPAddr{};
	localUDPAddr.sin_family = AF_INET;
	localUDPAddr.sin_port = htons(commonPort);
	localUDPAddr.sin_addr.s_addr = INADDR_ANY;

	result = bind(sockUdp, reinterpret_cast<sockaddr*>(&localUDPAddr), sizeof(localUDPAddr));
	assert(result != SOCKET_ERROR);

	{
		// UDP ソケットのポート番号を取得する
		sockaddr_in bindedLocalUdpAddr{};
		int addrLength = sizeof(bindedLocalUdpAddr);
		ret = getsockname(sockUdp, reinterpret_cast<sockaddr*>(&bindedLocalUdpAddr), &addrLength);
		assert(ret == 0);


		USHORT port = ntohs(bindedLocalUdpAddr.sin_port);

		assert(commonPort == port && "ポート番号が不一致");
	};

	connected = Connect(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);
}

// デストラクタ
NetQueue::~NetQueue()
{
	// ソケットを閉じてWSACleanup
	if (sockTcp != INVALID_SOCKET) {
		::closesocket(sockTcp);
		sockTcp = INVALID_SOCKET;
	}
	if (sockUdp != INVALID_SOCKET) {
		::closesocket(sockUdp);
		sockUdp = INVALID_SOCKET;
	}

	WSACleanup();
}

// 送信キューに積む
void NetQueue::Send(const std::string& content, TCP_OR_UDP tcp_or_udp)
{
	//キューに積む先がTCPかUDPを判別
	if (tcp_or_udp == TCP)
	{
		sendQueueTCP.push(content);
	}
	else if (tcp_or_udp == UDP)
	{
		sendQueueUDP.push(content);
	}
}

bool NetQueue::Connect(const char* ip, uint16_t port)
{
	//SOCKADDR_IN serverSocketAddress;
	SOCKADDR_IN server{};
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &server.sin_addr.s_addr) != 1)
	{
		std::cout << "inet_pton failed: ip=" << ip << std::endl;
		connected = false;
		return false;
	}

	bool connectUDPResult = (sockUdp != INVALID_SOCKET) ? ConnectImplUDP(server, sockUdp) : false;
	bool connectTCPResult = (sockTcp != INVALID_SOCKET) ? ConnectImplTCP(server, sockTcp) : false;

	if (!connectTCPResult)
	{
		int err = WSAGetLastError();
		std::cout << "TCP connect failed. WSA=" << err << std::endl;
	}
	if (!connectUDPResult)
	{
		int err = WSAGetLastError();
		std::cout << "UDP connect (peer set) failed. WSA=" << err << std::endl;
	}

	connected = connectTCPResult;        // ★ 通信の主軸がTCPなら、TCP成功をもって接続状態とする
	return  connectUDPResult && connectTCPResult;    // 必要ならUDP失敗でも true に緩めることも可能
}

//std::string NetQueue::ExtractHeadFromJsonString(std::string_view s)
//{
//	// "head" キーを探す
//	constexpr std::string_view key = "\"head\"";
//	auto keyPos = s.find(key);
//	if (keyPos == std::string_view::npos) return {};
//
//	// ':' を探す（key の直後から）
//	auto colonPos = s.find(':', keyPos + key.size());
//	if (colonPos == std::string_view::npos) return {};
//
//	// ':' の後の最初のダブルクォートを探す（値の開始）
//	auto quotePos = s.find('"', colonPos + 1);
//	if (quotePos == std::string_view::npos) return {};
//
//	// 値の開始位置（quotePos + 1）から、終了の非エスケープのダブルクォートを探す
//	std::string result;
//	result.reserve(32);
//
//	bool escaping = false;
//	for (size_t i = quotePos + 1; i < s.size(); ++i)
//	{
//		char c = s[i];
//		if (escaping)
//		{
//			// 最小限のエスケープ処理
//			switch (c)
//			{
//			case '"':  result.push_back('"'); break;
//			case '\\': result.push_back('\\'); break;
//			case '/':  result.push_back('/'); break;
//			case 'b':  result.push_back('\b'); break;
//			case 'f':  result.push_back('\f'); break;
//			case 'n':  result.push_back('\n'); break;
//			case 'r':  result.push_back('\r'); break;
//			case 't':  result.push_back('\t'); break;
//			default:
//				// \uXXXX 等はここでは扱わない（必要なら拡張）
//				result.push_back(c);
//				break;
//			}
//			escaping = false;
//		}
//		else
//		{
//			if (c == '\\')
//			{
//				escaping = true;
//			}
//			else if (c == '"')
//			{
//				// 終端クォートに到達
//				return result;
//			}
//			else
//			{
//				result.push_back(c);
//			}
//		}
//	}
//
//	// 終端クォートが見つからなければ空文字を返す
//	return {};
//}

// TagNameで検索し、見つかったら bodyを返す
json NetQueue::Find(std::string TagName)
{
	for (auto itr = RecvList.begin(); itr != RecvList.end(); )
	{
		if (itr->head == TagName)
		{
			json tmp = itr->body;
			itr = RecvList.erase(itr);
			return tmp;
		}
		else
		{
			++itr;
		}
	}
	// 見つからない場合は空の json を返す
	return json();
}

// 更新
void NetQueue::Update()
{
	if (connected == false)
	{
		return;  // 未接続なら無視
	}

	// TODO:
	//printfDx("send reqest TCP:%d", sendQueueTCP.size());

	// TCPから送信処理
	while (sendQueueTCP.empty() == false)
	{
		const std::string& front = sendQueueTCP.front();

		u_long bodySize = front.size() + 1;
		u_long headSize = sizeof(u_long);
		u_long totalSize = bodySize + headSize;
		u_long totalSizeNL = htonl(totalSize);

		std::vector<char> buffer(totalSize, 0x00);
		memcpy(buffer.data(), reinterpret_cast<const void*>(&totalSizeNL), headSize);
		memcpy(buffer.data() + headSize, front.c_str(), bodySize);

		if (int ret = send(sockTcp, buffer.data(), buffer.size(), 0);
			ret <= 0)
		{
			break;  // 送信失敗ならもう一度
		}

		//if (int ret = send(sockTcp, reinterpret_cast<const char*>(&totalSizeNL), headSize, 0);
		//	ret <= 0)
		//{
		//	continue;  // 送信失敗ならもう一度
		//}

		//if (int ret = send(sockTcp, front.c_str(), bodySize, 0);
		//	ret <= 0)
		//{
		//	break;
		//	//continue;  // 送信失敗ならもう一度
		//}

		sendQueueTCP.pop();
	}

	// TODO:
	// printfDx(" UDP:%d\n", sendQueueUDP.size());

	// UDPから送信処理
	while (sendQueueUDP.empty() == false)
	{
		const std::string& front = sendQueueUDP.front();

		// TODO:
		//printfDx("UDPSend:%s\n", front.c_str());

		u_long bodySize = front.size() + 1;
		u_long headSize = sizeof(u_long);
		u_long totalSize = bodySize + headSize;
		u_long totalSizeNL = htonl(totalSize);

		// バッファを作ってそこに書き込む
		std::vector<char> buffer(totalSize, 0x00);
		memcpy(buffer.data(), reinterpret_cast<const void*>(&totalSizeNL), headSize);
		memcpy(buffer.data() + headSize, front.c_str(), bodySize);

		if (int ret = send(sockUdp, buffer.data(), buffer.size(), 0);
			ret <= 0)
		{
			continue;  // 送信失敗ならもう一度
		}

		sendQueueUDP.pop();
	}

	// TCPから受信処理
	while (true)
	{
		// TCPの場合は１回のrecvですべてのデータが届くとは限らない
		// 読んでいるモードを切り替えながら読んでいく

		// 読んでいるモード
		enum Reading
		{
			AT_HEAD,
			AT_BODY,
		};

		static const u_long HEAD_SIZE = sizeof(u_long);

		static std::vector<char> bodyBuffer;
		static u_long totalSizeNL = 0;
		
		static Reading reading = AT_HEAD;
		static char* pWriteAt = nullptr;
		static size_t unreadSize = 0;

		if (unreadSize == 0)  // スイッチした初期化処理
		{
			switch (reading)
			{
			case AT_HEAD:
				pWriteAt = reinterpret_cast<char*>(&totalSizeNL);
				unreadSize = HEAD_SIZE;
				break;
			case AT_BODY:
				pWriteAt = bodyBuffer.data();
				unreadSize = bodyBuffer.size() - 1;
				break;
			default:
				assert(false && "未実装の読み取りモード");
				break;
			}
		}


		int ret = recv(sockTcp, pWriteAt, unreadSize, 0);
		if (ret == 0)  // 正常終了
		{
			connected = false;
			return;  // 切断
		}
		else if (ret < 0)  // 異常
		{
			int errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK)  // ノンブロッキング処理で無視
			{
				break;  // 現状TCPから受信内容なしのためループ離脱
			}
			else
			{
				connected = false;
				return;  // 切断
			}
		}
		else  // 受信した
		{
			unreadSize -= ret;
			pWriteAt += ret;

			if (unreadSize == 0)  // 全部読み込めた
			{
				switch (reading)
				{
				case AT_HEAD:
				{
					// 全部読めたのがheadなら サイズ確定とbodyバッファの準備をする
					u_long totalSize = ntohl(totalSizeNL);
					bodyBuffer.resize(totalSize - HEAD_SIZE, 0x00);
					// 念の為0埋め
					std::fill(bodyBuffer.begin(), bodyBuffer.end(), 0x00);

					reading = AT_BODY;
					break;
				}
				case AT_BODY:
				{
					// 全部読めたのがbodyなら 受信完了 受信リストに追加処理

					std::string_view jsonStr = bodyBuffer.data();

					// TODO:
					// Logger::WriteOut(jsonStr);

					json bodyJson = json::parse(bodyBuffer.begin(), bodyBuffer.end());

					// TODO:
					//printfDx("TCP受信:%s\n", jsonStr.data());

					std::string head = bodyJson.value("head", "undefined");
					RecvList.push_back(Recv{ head, bodyJson.at("content") });

					reading = AT_HEAD;
					break;
				}
				default:
					assert(false && "未実装の読み取りモード");
					break;
				}
			}
			else  // 読み足らず...
			{
				continue;
			}
		}
	}

	// UDPから受信処理
	while (true)
	{
		// UDPの場合は１回ですべて受け取る必要がある。
		static std::array<char, BUFFER_SIZE> buffer{};
		std::fill(buffer.begin(), buffer.end(), 0x00);
		int ret = recv(sockUdp, buffer.data(), buffer.size(), 0);
		if (ret == 0)  // 正常終了
		{
			connected = false;
			return;  // 切断
		}
		else if (ret < 0)  // 異常
		{
			int errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK)  // ノンブロッキング処理で無視
			{
				break;  // UDPから受信内容なしのためループ離脱
			}
			else
			{
				connected = false;
				return;  // 切断
			}
		}
		else  //　受信した
		{
			if (ret < sizeof(u_long))
			{
				assert(ret >= sizeof(u_long)
					&& "総サイズのヘッドの分がない");
				continue;  // 受信失敗のため今回は破棄しつつ次を受信
			}

			u_long totalSizeNL = 0UL;
			memcpy(&totalSizeNL, buffer.data(), sizeof(totalSizeNL));

			u_long totalSize = ntohl(totalSizeNL);
			if (totalSize != ret)
			{
				assert(totalSize == ret
					&& "受信したサイズとバッファ内指定の総サイズが不一致");
				continue;  // 受信失敗のため今回は破棄しつつ次を受信
			}

			// 受信バッファをjson文字列にして受信リストに追加
			u_long headSize = sizeof(u_long);
			std::string_view jsonStr = buffer.data() + headSize;

			// TODO:
			//printfDx("UDP受信:%s\n", jsonStr.data());

			json bodyJson = json::parse(jsonStr);

			std::string head = bodyJson.value("head", "undefined");
			RecvList.push_back(Recv{ head, bodyJson.at("content") });
		}
	}
}

// 描画(もしかして必要ない？) ← 必要ないです
void NetQueue::Draw()
{

}

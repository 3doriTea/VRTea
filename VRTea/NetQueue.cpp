#include "NetQueue.h"
#include <winsock2.h>  

// ノンブロッキング設定(そのまま次の処理に行く用)
void NetQueue::SetNonBlocking(SOCKET S)
{
    u_long mode = 1; 
    ioctlsocket(S, FIONBIO, &mode);
}

// コンストラクタ(ここは前回のネットワークの物を引用)
NetQueue::NetQueue()
{
    // WinSock2.2 初期化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Error : WSAStartup" << std::endl;
        return;
    }
    std::cout << "Success : WSAStartup" << std::endl;

    //リスンソケットの作成(前回の応用)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "Error : socket" << std::endl;
        WSACleanup();
        return;
    }
    //set_nonblocking(sock);  // TOOD: ノンブロッキングのところ
    std::cout << "Success : socket" << std::endl;

    // ノンブロッキング処理
    SetNonBlocking(sock);

}

// デストラクタ
NetQueue::~NetQueue()
{
    // ソケットを閉じてWSACleanup
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    WSACleanup();
}

// 送信キューに積む
void NetQueue::Send(const std::string& content)
{
    // TCPのメッセージ境界を作るために、改行
    sendQueue.push(content + "\n");
}

bool NetQueue::Connect(const char* ip, uint16_t port)
{
    return false;
}

// 受信キューから1件取り出す処理
std::string NetQueue::Read(std::string& out)
{
    std::string Q = readQueue.front();
    readQueue.pop();

    return Q;
}

void NetQueue::Update()
{
    std::string sendData;

    int ret = send(sock, 
        sendData.c_str(),
        (sendData.size()),  // 送る文字列のサイズ
        0);                   
}

void NetQueue::Draw()
{

}

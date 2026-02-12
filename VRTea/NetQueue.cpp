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
    : sockUDP(INVALID_SOCKET), sockTCP(INVALID_SOCKET) // 初期化の警告が出るので修正
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
void NetQueue::Send(const std::string& content, TCP_OR_UDP)
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
    if (readQueue.empty())
    {
        out.clear();
        return std::string();
    }
    std::string Q = std::move(readQueue.front()); // データを移動してもらう
    readQueue.pop();
    out = Q;

    return Q;
}

// TagNameで検索し、見つかったら bodyを返す
json NetQueue::Find(std::string TagName)
{
    for (const auto& r : RecvList)
    {
        if (r.head == TagName)
        {
            return r.body; // TagNameが合ったらbodyを返す
        }
    }
    // 見つからない場合は空の json を返す
    return json();
}

// 更新
void NetQueue::Update()
{
    if (!connected || sock == INVALID_SOCKET) return;

    // 送信中バッファが空なら、送信キューから次のデータを取る
    if (sendingBuffer.empty() && !sendQueue.empty())
    {
        sendingBuffer = std::move(sendQueue.front());
        sendQueue.pop();
    }

    // 受信処理
    char temp[4096]{};
    std::string sendData;
    
    for (;;) // 条件が満たされない限り、処理が無限に繰り返される
    {
        // データを送信
        int ret = send(sock,
            sendData.c_str(),
            static_cast<int>(sendData.size()),  // 送る文字列のサイズ(警告をなくすためintに変換)
            0);
        if (ret > 0)
        {
            for (;;)
            {
                size_t pos = recvBuffer.find('\n');
                if (pos == std::string::npos) break;

                std::string msg = recvBuffer.substr(0, pos);
                recvBuffer.erase(0, pos + 1);

                readQueue.push(std::move(msg));
            }
        }
        else if (ret == 0)
        {
            // 相手が切断
            connected = false;
            break;
        }
        else
        {
            int err = WSAGetLastError();
            if (err == WSAECONNRESET)
            {
                // 今は受信データ無し
                break;
            }
            // その他のエラー
            connected = false;
            break;
        }
    }
}

// 描画(もしかして必要ない？)
void NetQueue::Draw()
{

}

#include "NetQueue.h"
#include <winsock2.h>  
#include <nlohmann/json.hpp>
using nlohmann::json;

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

    // リスンソケットの作成(前回の応用)
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
  
    for (;;) // 条件が満たされない限り、処理が無限に繰り返される
    {
        // データを送信
        int ret = send(sock,
            temp,
            static_cast<int>(sizeof(temp)),  // 送る文字列のサイズ(警告をなくすためintに変換)
            0);
        if (ret > 0)
        {
            // 受信分を末尾に追加
            recvBuffer.append(temp, ret);

            for (;;)
            {
                // 4バイトの長さヘッダがあるか
                if (recvBuffer.size() < 4) break;

                uint32_t len_net = 0;
                std::memcpy(&len_net, recvBuffer.data(), 4);
                const uint32_t len_host = ntohl(len_net);
             
                // 本体が揃っているか？
                if (recvBuffer.size() < 4u + static_cast<size_t>(len_host)) break;
                
                // フレームを取り出す
                std::string payload = recvBuffer.substr(4, len_host);
                recvBuffer.erase(0, 4u + static_cast<size_t>(len_host));

                // 従来の readQueue にも積む
                readQueue.push(payload);

                // JSON を head/body に分配して RecvList へ (AIを参照)
                try
                {
                    json j = json::parse(payload);

                    std::string head;
                    if (j.contains("head") && j["head"].is_string()) {
                        head = j["head"].get<std::string>();
                    }
                    else if (j.contains("tag") && j["tag"].is_string()) {
                        head = j["tag"].get<std::string>(); // 別名対応
                    }

                    json body;
                    if (j.contains("body")) {
                        body = j["body"];
                    }
                    else if (j.contains("data")) {
                        body = j["data"];
                    }
                    else {
                        body = j; // そのまま格納（単純メッセージ等）
                    }

                    RecvList.push_back(Recv{ head, body });
                }
                catch (const std::exception& e)
                {
                    // パース失敗：生データとエラー内容を body に入れて head="" として格納
                    json err;
                    err["raw"] = payload;
                    err["parse_error"] = e.what();
                    RecvList.push_back(Recv{ "", err });
                }
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

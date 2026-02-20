#include "NetQueue.h"
#include <winsock2.h>  
#include <assert.h>
#include <nlohmann/json.hpp>
using nlohmann::json;

bool ConnectImpl(const SOCKADDR_IN& sockAddrIn, SOCKET sock)
{
    int serverAddressLength = sizeof(sockAddrIn);
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    int ret = connect(sock, (SOCKADDR*)&sockAddrIn, sizeof(sockAddrIn));
    if (ret == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int result = select(0, nullptr, &fds, nullptr, &timeout);
        if (result > 0 && FD_ISSET(sock, &fds))
        {
            int err = 0;
            socklen_t len = sizeof(err);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
            if (err != 0)
            {
                // 失敗
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

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

    sockUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(sockUDP != INVALID_SOCKET && "Error : socketUDP");

    sockTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(sockTCP != INVALID_SOCKET && "Error : listenSock");

    unsigned long arg = 0x01;
    int result = ioctlsocket(sockTCP, FIONBIO, &arg);
    assert(result != SOCKET_ERROR);
    
    const char* ip = "192.168.42.5";
    uint16_t port = 3000;
    connected = Connect(ip,port);
}

// デストラクタ
NetQueue::~NetQueue()
{
    // ソケットを閉じてWSACleanup
    
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
    SOCKADDR_IN serverSocketAddress;
    // 0でクリア
    memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverSocketAddress.sin_addr.s_addr);

    bool connectUDPResult = ConnectImpl(serverSocketAddress, sockUDP);
    assert(connectUDPResult);
    bool connectTCPResult = ConnectImpl(serverSocketAddress, sockTCP);
    assert(connectTCPResult);
    
    return connectUDPResult && connectTCPResult;
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
    if (!connected == sockUDP == INVALID_SOCKET) return;

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
        int ret = send(sockUDP,
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

using Newtonsoft.Json.Linq;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using VRTeaServer.Logging;

namespace VRTeaServer.Service
{
	/// <summary>
	/// ボットのサービス
	/// TODO: ボットタスクの人が実装してください
	/// </summary>
	internal class BotService : IService
	{
		private IPAddress _serverIPAddress;  // サーバーのIPアドレス
		private ushort _gamePort;            // ゲームサービスを公開するポート番号
		
		public BotService(IPAddress serverIPAddress, ushort gamePort)
		{
			_serverIPAddress = serverIPAddress;
			_gamePort = gamePort;
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(async () =>
		{
			using var udp = new UdpClient();
			using var tcp = new TcpClient();

			// MEMO: UdpClient の Connectメソッドは見かけ上の接続で、
			//     : 送信先をあらかじめ指定しておけるだけ
			udp.Connect(_serverIPAddress, _gamePort);

			// MEMO: 一方の TcpClientはサーバーと接続するため、 awaitする
			await tcp.ConnectAsync(_serverIPAddress, _gamePort);

			ConcurrentQueue<SendData> tcpSendQueue = [];
			ConcurrentQueue<ReceiveData> tcpReceiveQueue = [];

			void BotSend(string content)
			{
				SendData.FromString(content, out var data);
				tcpSendQueue.Enqueue(data);
			}

			void BotLife()
			{
				while (true)
				{
					while (tcpReceiveQueue.TryDequeue(out var data))
					{
						JObject? json = null;
						try
						{
							json = JObject.Parse(data.GetString());
						}
						catch (Exception ex)
						{
							Log.WriteLine($"BOT:jsonパースエラー発生:\r\n{ex}");
							continue;
						}

						data.GetString();
						var head = json.GetValue("head")?.ToString() ?? "";

						if (head != "Event")
						{
							continue;  // イベント以外は無視
						}

						// content.head を取得していく
						var eventContent = json["content"];
						if (eventContent is null)
						{
							Log.WriteLine($"BOT: 受け取ったイベントに contentが含まれていない");
							continue;
						}
						var eventContentHead = eventContent["head"];
						if (eventContentHead is null)
						{
							Log.WriteLine($"BOT: 受け取ったイベントに content.headが含まれていない");
							continue;
						}

						string? eventContentHeadStr = eventContentHead.Value<string>();
						if (eventContentHeadStr is null)
						{
							Log.WriteLine($"BOT: 受け取ったイベントの content.headを stringに変換できない");
							continue;
						}

						// クライアントから送信され、サーバー経由でゲットしたチャットコンテンツ
						var chatContent = json.Value<string>();

						Log.WriteLine($"BOT: Chat受け取った「{chatContent ?? "{{null}}"}」");

						if (chatContent is null)
						{
							// チャットコンテンツがないよ！
							Log.WriteLine($"BOT: chatContentが nullだった");
							continue;
						}

						// TODO: わかるー　以外も追加する
						JObject sendJson = JObject.FromObject(new
						{
							head = "Event",
							content = new
							{
								head = "Chat",
								content = $"わかるー",
							},
						});

						BotSend($"{sendJson}");
					}
				}
			}

			async Task BotSendCycle()
			{
				while (true)
				{
					SendData sendData = await _sessionManager.SendDequeue(sessionId, cts);
					await stream.WriteAsync(sendData.Buffer, cts.Token);
				}
			}

			await Task.WhenAll(
				Task.Run(async () =>  // tcp送信サイクル
				{
					int bytesRead = 0;
					while ((bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length, cts.Token)) > 0)
					{
						await _sessionManager.ReceiveEnqueue(sessionId, new ReceiveData(buffer.AsSpan(0..bytesRead).ToArray()), cts);
					}
				}, cts.Token),
				Task.Run(async () =>  // tcp受信サイクル
				{
					while (true)
					{
						SendData sendData = await _sessionManager.SendDequeue(sessionId, cts);
						await stream.WriteAsync(sendData.Buffer, cts.Token);
					}
				}, cts.Token));
		});
	}
}

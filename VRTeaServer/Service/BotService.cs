using Newtonsoft.Json.Linq;
using System;
using System.Buffers.Binary;
using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;
using VRTeaServer.Logging;
using static VRTeaServer.Utility.NetWorkUtil;

namespace VRTeaServer.Service
{

	/// <summary>
	/// ボットのサービス
	/// TODO: ボットタスクの人が実装してください
	/// </summary>
	internal class BotService : IService
	{
		const int BufferSize = 1024 * 4;
		
		private IPAddress _serverIPAddress;  // サーバーのIPアドレス
		private ushort _gamePort;            // ゲームサービスを公開するポート番号
		
		public BotService(IPAddress serverIPAddress, ushort gamePort)
		{
			_serverIPAddress = serverIPAddress;
			_gamePort = gamePort;
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(async () =>
		{
			var localIPEP = new IPEndPoint(IPAddress.Any, 0);
			using var tcp = new TcpClient(localIPEP);
			using var udp = new UdpClient(localIPEP);

			Log.WriteLine($"ボット起動した:{localIPEP}");

			try
			{
				await Task.Delay(1000, cts.Token);
			}
			catch (OperationCanceledException)
			{
				return;  // ボット参加待機中にキャンセル
			}
			Log.WriteLine($"ボットこれから参加する");

			// MEMO: UdpClient の Connectメソッドは見かけ上の接続で、
			//     : 送信先をあらかじめ指定しておけるだけ
			udp.Connect(_serverIPAddress, _gamePort);

			// MEMO: 一方の TcpClientはサーバーと接続するため、 awaitする
			await tcp.ConnectAsync(_serverIPAddress, _gamePort);

			

			ConcurrentQueue<SendData> tcpSendQueue = [];
			ConcurrentQueue<ReceiveData> tcpReceiveQueue = [];

			

			async Task RunTcpCycleSend() => await Task.Run(async () =>
			{
				Log.WriteLine($"ボットTCP送信処理開始");
				var stream = tcp.GetStream();
				while (true)
				{
					try
					{
						while (tcpSendQueue.TryDequeue(out var data))
						{
							await stream.WriteAsync(data.Buffer, cts.Token);
						}
					}
					catch (OperationCanceledException)
					{
						Log.WriteLine($"ボットTCP送信処理キャンセル受信");
						break;
					}
					catch (Exception ex)
					{
						Log.Error($"ボットTCP送信処理例外:{ex}");
					}
				}
				Log.WriteLine($"ボットTCP送信処理停止");
			});

			async Task RunTcpCycleReceive() => await Task.Run(async () =>
			{
				Log.WriteLine($"ボットTCP受信処理開始");
				var stream = tcp.GetStream();
				while (true)
				{
					try
					{
						var buffer = new byte[BufferSize];
						int read = await stream.ReadAsync(buffer, cts.Token);
						tcpReceiveQueue.Enqueue(new ReceiveData(buffer[..read]));
					}
					catch (OperationCanceledException)
					{
						Log.WriteLine($"ボットTCP受信処理キャンセル受信");
						break;
					}
					catch (Exception ex)
					{
						Log.Error($"ボットTCP受信処理例外:{ex}");
					}
				}
				Log.WriteLine($"ボットTCP受信処理停止");
			});

			async Task RunUdpCycle() => await Task.Run(async () =>
			{
				Log.WriteLine($"ボットUDP送受信処理開始");
				while (true)
				{
					try
					{
						var json = JObject.FromObject(new
						{
							head = "Update",
							content = new
							{
								position = new
								{
									x = 0, y = 0, z = 0,
								},
							},
						});

						SendData.FromString($"{json}", out var sendData);
						await udp.SendAsync(sendData.Buffer, cts.Token);

						_ = await udp.ReceiveAsync(cts.Token);
					}
					catch (OperationCanceledException)
					{
						Log.WriteLine($"ボットUDP送受信処理キャンセル受信");
						break;
					}
					catch (Exception ex)
					{
						Log.Error($"ボットUDP送受信処理例外:{ex}");
					}
				}
				Log.WriteLine($"ボットUDP送受信処理停止");
			});
			
			async Task RunBotLife() => await Task.Run(() =>
			{
				Log.WriteLine($"ボットライフ開始");
				while (true)
				{
					try
					{

					}
					catch (OperationCanceledException)
					{
						Log.WriteLine($"ボットライフキャンセル受信");
						break;
					}
					catch (Exception ex)
					{
						Log.Error($"ボットライフ例外:{ex}");
					}
				}
				Log.WriteLine($"ボットライフ停止");
			});
			
			await Task.WhenAll(
				RunTcpCycleSend(),
				RunTcpCycleReceive(),
				RunUdpCycle(),
				RunBotLife());

			Log.WriteLine($"ボット停止した");
#if false
			//async Task BotSend(string content)
			//{
			//	await Task.Run(() =>
			//	{
			//		SendData.FromString(content, out var data);
			//		tcpSendQueue.Enqueue(data);
			//	});
			//}

			//async Task BotLife()
			//{
			//	Log.WriteLine($"ボットライフ起動");

			//	JObject firstJoinJson = JObject.FromObject(new
			//	{
			//		head = "Join",
			//	});

			//	await BotSend($"{firstJoinJson}");

			//	while (true)
			//	{
			//		while (tcpReceiveQueue.TryDequeue(out var data))
			//		{
			//			JObject? json = null;
			//			try
			//			{
			//				json = JObject.Parse(data.GetString());
			//			}
			//			catch (Exception ex)
			//			{
			//				Log.WriteLine($"BOT:jsonパースエラー発生:\r\n{ex}");
			//				continue;
			//			}

			//			data.GetString();
			//			var head = json.GetValue("head")?.ToString() ?? "";

			//			if (head != "Event")
			//			{
			//				continue;  // イベント以外は無視
			//			}

			//			// content.head を取得していく
			//			var eventContent = json["content"];
			//			if (eventContent is null)
			//			{
			//				Log.WriteLine($"BOT: 受け取ったイベントに contentが含まれていない");
			//				continue;
			//			}
			//			var eventContentHead = eventContent["head"];
			//			if (eventContentHead is null)
			//			{
			//				Log.WriteLine($"BOT: 受け取ったイベントに content.headが含まれていない");
			//				continue;
			//			}

			//			string? eventContentHeadStr = eventContentHead.Value<string>();
			//			if (eventContentHeadStr is null)
			//			{
			//				Log.WriteLine($"BOT: 受け取ったイベントの content.headを stringに変換できない");
			//				continue;
			//			}

			//			// クライアントから送信され、サーバー経由でゲットしたチャットコンテンツ
			//			var chatContent = json.Value<string>();

			//			Log.WriteLine($"BOT: Chat受け取った「{chatContent ?? "{{null}}"}」");

			//			if (chatContent is null)
			//			{
			//				// チャットコンテンツがないよ！
			//				Log.WriteLine($"BOT: chatContentが nullだった");
			//				continue;
			//			}

			//			// TODO: わかるー　以外も追加する
			//			JObject sendJson = JObject.FromObject(new
			//			{
			//				head = "Event",
			//				content = new
			//				{
			//					head = "Chat",
			//					content = $"{chatContent} <- わかるー",
			//				},
			//			});

			//			await BotSend($"{sendJson}");
			//		}

			//	}
			//}

			//async Task BotSendTask(NetworkStream stream)
			//{
			//	while (true)
			//	{
			//		try
			//		{
			//			while (tcpSendQueue.TryDequeue(out var data))
			//			{
			//				await stream.WriteAsync(data.Buffer, cts.Token);
			//			}
			//		}
			//		catch (OperationCanceledException)
			//		{
			//			break;
			//		}
			//	}
			//}

			//async Task BotReceiveTask(NetworkStream stream)
			//{
			//	while (true)
			//	{
			//		byte[] totalSizeBuffer = new byte[sizeof(int)];
			//		if (!await ReadExactlyAsync(stream, totalSizeBuffer, cts))
			//		{
			//			return;  // 切断されたら終了
			//		}

			//		int totalSize = BinaryPrimitives.ReadInt32BigEndian(totalSizeBuffer);
			//		int bodySize = totalSize - sizeof(int);

			//		if (bodySize <= 0)
			//		{
			//			return;  // ボディサイズがおかしいなら無視
			//		}

			//		byte[] bodyBuffer = new byte[bodySize];
			//		if (!await ReadExactlyAsync(stream, bodyBuffer, cts))
			//		{
			//			return;  // 切断されたら終了
			//		}

			//		// 2つを合わせたバッファを作成
			//		byte[] combined = new byte[totalSize];
			//		totalSizeBuffer.AsSpan().CopyTo(combined.AsSpan().Slice(0, totalSizeBuffer.Length));
			//		bodyBuffer.AsSpan().CopyTo(combined.AsSpan().Slice(totalSizeBuffer.Length));

			//		tcpReceiveQueue.Enqueue(new ReceiveData(combined));
			//	}
			//}

			//await Task.WhenAll(
			//	BotReceiveTask(tcp.GetStream()),
			//	BotSendTask(tcp.GetStream()),
			//	BotLife(),
			//	Task.Run(() =>
			//	{

			//	}, cts.Token));


			//Log.WriteLine($"ボット停止した");
			//await Task.WhenAll(
			//	Task.Run(async () =>  // tcp送信サイクル
			//	{
			//		int bytesRead = 0;
			//		while ((bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length, cts.Token)) > 0)
			//		{
			//			await _sessionManager.ReceiveEnqueue(sessionId, new ReceiveData(buffer.AsSpan(0..bytesRead).ToArray()), cts);
			//		}
			//	}, cts.Token),
			//	Task.Run(async () =>  // tcp受信サイクル
			//	{
			//		while (true)
			//		{
			//			SendData sendData = await _sessionManager.SendDequeue(sessionId, cts);
			//			await stream.WriteAsync(sendData.Buffer, cts.Token);
			//		}
			//	}, cts.Token));
#endif
		});
	}
}

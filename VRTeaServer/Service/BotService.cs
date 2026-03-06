using Newtonsoft.Json.Linq;
using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;
using VRTeaServer.AI;
using VRTeaServer.Logging;

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
		private AIBrain? _ai;                // AIの脳
		
		public BotService(IPAddress serverIPAddress, ushort gamePort, string aiToken)
		{
			_serverIPAddress = serverIPAddress;
			_gamePort = gamePort;

			if (!string.IsNullOrEmpty(aiToken))
			{
				_ai = new AIBrain(aiToken);
			}
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

			int botSessionId = -1;
			uint botSelfColor = 0;
			float positionX = 0.0f;
			float positionY = 0.0f;
			float positionZ = 0.0f;

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
									x = positionX,
									y = positionY,
									z = positionZ,
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

			async Task<string> BotReadAIOutput(string content) => await Task.Run(async () =>
			{
				int embedBegin = content.IndexOf("##");
				if (embedBegin == -1)
				{
					return content;
				}

				int embedEnd = content.IndexOf("##", embedBegin + 1);
				if (embedEnd == -1)
				{
					return content.Replace("##", "");
				}

				ReadOnlySpan<char> colorCodeStr = content.AsSpan(embedBegin + 2, embedEnd - embedBegin - 2);

				Log.WriteLine($"色コードのところ{colorCodeStr}");

				if (uint.TryParse(
					colorCodeStr,
					System.Globalization.NumberStyles.HexNumber,
					null,
					out var color))
				{
					await BotChangeColor(color);
				}

				return content.Remove(embedBegin, embedEnd - embedBegin + 2);
			});

			async Task BotChangeColor(uint color) => await Task.Run(() =>
			{
				JObject sendJson = JObject.FromObject(new
				{
					head = "Event",
					content = new
					{
						head = "NewColor",
						content = color,
					},
				});
				SendData.FromString($"{sendJson}", out var sendData);

				tcpSendQueue.Enqueue(sendData);
			});

			async Task RunBotLife() => await Task.Run(async () =>
			{
				Log.WriteLine($"ボットライフ開始");

				JObject firstJoinJson = JObject.FromObject(new
				{
					head = "Join",
				});
				SendData.FromString($"{firstJoinJson}", out var firstJoinData);
				tcpSendQueue.Enqueue(firstJoinData);

				while (true)
				{
					try
					{
						if (tcpReceiveQueue.TryDequeue(out var receiveData))
						{
							var receiveJson = JObject.Parse(receiveData.GetString());

							var head = receiveJson.Value<string>("head");
							if (head == "Joined")
							{
								var content = receiveJson["content"];
								if (content is null)
								{
									Log.Error($"ボットライフ:{"参加情報なのにコンテンツがない"}");
									continue;
								}

								botSessionId = content.Value<int>("id");
								botSelfColor = content.Value<uint>("color");
								Log.WriteLine($"ボットライフ:私のIdは{botSessionId}です。");
							}
							else if (head == "Event")
							{
								var content = receiveJson["content"];
								if (content is null)
								{
									Log.Error($"ボットライフ:{"イベントなのにコンテンツがない"}");
									continue;
								}

								if (content.Value<string>("head") != "Chat")
								{
									Log.Error($"ボットライフ:{"受信イベントがチャット以外対応していない"}");
									continue;
								}

								var chatContent = content["content"];
								if (chatContent is null)
								{
									Log.Error($"ボットライフ:{"チャットイベントなのにコンテンツがない"}");
									continue;
								}

								var chatContentMessage = chatContent.Value<string>("message");
								if (chatContentMessage is null)
								{
									Log.Error($"ボットライフ:{"チャットコンテンツのメッセージがない"}");
									continue;
								}
								var chatContentSenderId = chatContent.Value<int>("senderId");
								if (chatContentSenderId == 0)
								{
									Log.Error($"ボットライフ:{"チャットコンテンツの送信者Idがない"}");
									continue;
								}
								var chatContentSenderName = chatContent.Value<string>("sender");
								if (chatContentSenderName is null)
								{
									Log.Error($"ボットライフ:{"チャットコンテンツの送信者名がない"}");
									continue;
								}

								if (chatContentSenderId == botSessionId
									|| chatContentSenderId == -1)
								{
									_ai?.AddHistory("assistant", chatContentMessage);
									continue;  // 自分自身で送ったチャット/ワールドメッセージなら無視
								}

								string? replyContent;
								if (_ai is null)
								{
									replyContent = $"{chatContentMessage} <- わかるー";
								}
								else
								{
									string? output = await _ai.Ask(chatContentMessage);

									if (string.IsNullOrEmpty(output))
									{
										replyContent = $"{chatContentMessage} <- わかるー";
									}
									else
									{
										replyContent = await BotReadAIOutput(output);
									}
								}

								JObject sendJson = JObject.FromObject(new
								{
									head = "Event",
									content = new
									{
										head = "Chat",
										content = replyContent,
									},
								});
								SendData.FromString($"{sendJson}", out var sendData);

								tcpSendQueue.Enqueue(sendData);
							}
							else
							{
								Log.Error($"ボットライフ:{$"知らないTCP通信{receiveJson}"}");
							}
						}

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
		});
	}
}

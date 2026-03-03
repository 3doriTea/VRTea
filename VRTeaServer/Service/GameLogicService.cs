using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using VRTeaServer.GameLogic;
using VRTeaServer.Logging;
using VRTeaServer.Exceptions;

namespace VRTeaServer.Service
{
	internal class GameLogicService : IService
	{
		private readonly SessionManager _sessionManager;
		
		/// <summary>
		/// プレイヤーとかが送信したテキストチャット
		/// </summary>
		private class TextChat
		{
			public string Content { get; set; } = string.Empty;
			public uint Id { get; set; } = uint.MinValue;
		}

		private readonly ConcurrentDictionary<int, PlayerStatus> playersStatus = [];
		private readonly ConcurrentDictionary<int, PlayerData> playersData = [];

		public GameLogicService(SessionManager sessionManager)
		{
			_sessionManager = sessionManager;
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(async () =>
		{
			async Task SendTo(int id, string content)
			{
				SendData.FromString(content, out var data);
				await _sessionManager.SendEnqueue(id, data, cts);
			}

			async Task SendToUDP(int id, string content)
			{
				SendData.FromString(content, out var data);
				await _sessionManager.SendEnqueueUDP(id, data, cts);
			}

			_sessionManager.OnDisconnected += (ipEndPoint, leavedId) =>
			{
				playersData.Remove(leavedId, out var data);
				if (data is null)
				{
					return;
				}

				playersStatus.Remove(leavedId, out var status);

				string leavedUserName = $"{data.Name}";

				JObject sendJson = JObject.FromObject(new
				{
					head = "Event",
					content = new
					{
						head = "Chat",
						content = new
						{
							senderId = -1,
							sender = "*",
							message = $"{leavedUserName}さんが退出しました。",
						},
					},
				});


				foreach (var sendId in _sessionManager.Sessions)
				{
					_ = SendTo(sendId, $"{sendJson}");
				}
			};

			void RequestProc(int sessionId, in ReceiveData data)
			{
				JObject? json = null;
				try
				{
					//Log.WriteLine($"受信した文字列:{data.GetString()}");
					//Log.WriteLine($"受信したBinary:{BitConverter.ToString(data.Buffer)}");
					json = JObject.Parse(data.GetString());
				}
				catch (Exception ex)
				{
					Log.WriteLine($"SID:{sessionId} からエラー発生:\r\n{ex}");
					return;
				}

				var head = json.GetValue("head")?.ToString() ?? "";

				switch (head)
				{
					case "Update":  // 毎フレームの更新処理
						var updateContent = json["content"];
						if (updateContent is null)
						{
							Log.WriteLine($"SID:{sessionId}に contentが含まれていない");
							break;
						}

						Update(sessionId, updateContent);

						break;
					case "Event":
						var eventContent = json["content"];
						if (eventContent is null)
						{
							Log.WriteLine($"SID:{sessionId}に contentが含まれていない");
							break;
						}
						var eventContentHead = eventContent["head"];
						if (eventContentHead is null)
						{
							Log.WriteLine($"SID:{sessionId}に content.headが含まれていない");
							break;
						}

						string? eventContentHeadStr = eventContentHead.Value<string>();
						if (eventContentHeadStr is null)
						{
							Log.WriteLine($"SID:{sessionId}の content.headを stringに変換できない");
							break;
						}

						switch (eventContentHeadStr)
						{
							case "Chat":
								EventChat(sessionId, eventContent);
								break;
							case "NewName":
								EventChangeName(sessionId, eventContent);
								break;
							case "NewColor":
								EventChangeColor(sessionId, eventContent);
								break;
							default:
								Log.WriteLine($"SID:{sessionId}から未実装のイベント:{eventContentHeadStr}を受信した");
								break;
						}
						break;
					case "Join":
						Join(sessionId);
						break;
					default:
						Log.WriteLine($"SID:{sessionId} から不明な head:{head}");
						break;
				}

				void Update(int sessionId, JToken updateJson)
				{
					// プレイヤーの状態を更新する
					playersStatus.TryGetValue(sessionId, out var status);
					if (status is null)
					{
						Log.Error($"[SID:{sessionId}] Updateに応答中, statusの取得に失敗");
						return;
					}

					JToken? position = updateJson["position"];
					if (position is null)
					{
						Log.WriteLine($"[SID:{sessionId}] Updateに応答中, positionが含まれていない");
						return;
					}

					var playerState = new PlayerStatus
					{
						PositionX = (float)(position["x"] ?? 0.0f),
						PositionY = (float)(position["y"] ?? 0.0f),
						PositionZ = (float)(position["z"] ?? 0.0f),
					};

					bool updated = playersStatus.TryUpdate(
						sessionId,
						playerState,
						status);

					// 更新失敗
					if (updated == false)
					{
						Log.WriteLine($"[SID:{sessionId}] statusの更新に失敗");
						return;
					}

					Dictionary<string, JObject> userNameToData = [];

					// 他プレイヤーの情報を返す
					foreach (var (pSId, pData) in playersData)
					{
						if (pSId == sessionId)
						{
							continue;  // 自分自身は無視
						}

						playersStatus.TryGetValue(pSId, out var pStat);
						if (pStat is null)
						{
							continue;
						}

						userNameToData.Add(pData.Name, JObject.FromObject(new
						{
							color = pData.Color,
							position = new
							{
								x = pStat.PositionX,
								y = pStat.PositionY,
								z = pStat.PositionZ,
							},
						}));
					}

					if (userNameToData.Count <= 0)
					{
						return;  // 名前とデータのペアがないならリターン
					}

					JObject sendJson = JObject.FromObject(new
					{
						head = "Updated",
						content = userNameToData,
					});

					_ = SendToUDP(sessionId, $"{sendJson}");
					//foreach (var sendId in _sessionManager.Sessions)
					//{
					//	// MEMO: 参加したてのIdさんを含む全員に送信する
					//	_ = SendTo(sendId, $"{sendJson}");
					//}
				}

				void EventChangeName(int sessionId, JToken changeContentJson)
				{
					//Log.WriteLine($"名前変更json:{changeContentJson}");
					var changedName = changeContentJson.Value<string>("content");

					Log.WriteLine($"[SID:{sessionId}]{changedName ?? "{{null}}"}");
					
					if (changedName is null)
					{
						// チャットコンテンツがないよ！
						return;
					}

					playersData.AddOrUpdate(
						sessionId,
						new PlayerData(),
						(sessionId, currentData) =>
						{
							currentData.Name = changedName;
							return currentData;
						});
					// TODO: ここで全員に色変更を送信すべきか、更新処理で毎回色も送っちゃうか
				}

				void EventChangeColor(int sessionId, JToken changeContentJson)
				{
					//Log.WriteLine($"受信した色変更jsonContent:{changeContentJson}");
					var changedColor = changeContentJson.Value<uint>("content");

					Log.WriteLine($"[SID:{sessionId}] changeColor: {changedColor:X8}");

					playersData.AddOrUpdate(
						sessionId,
						new PlayerData(),
						(sessionId, currentData) =>
						{
							currentData.Color = changedColor;
							return currentData;
						});
					// TODO: ここで全員に色変更を送信すべきか、更新処理で毎回色も送っちゃうか
				}

				void EventChat(int sessionId, JToken chatContentJson)
				{
					Log.WriteLine($"チャットのJSON:{chatContentJson}");
					// クライアントから送信されたチャットコンテンツ
					var chatContent = chatContentJson.Value<string>("content");

					Log.WriteLine($"[SID:{sessionId}]{chatContent ?? "{{null}}"}");

					if (chatContent is null)
					{
						// チャットコンテンツがないよ！
						return;
					}

					
					// 送信者名
					var senderName = string.Empty;

					// プレイヤーデータが取得できなかったらSIDだけでチャットする
					playersData.TryGetValue(sessionId, out var pData);
					if (pData is null)
					{
						senderName = $"[SID:{sessionId}]";
					}
					else
					{
						senderName = $"[{pData.Name}]";
					}

					// 送るチャットデータ構築
					var sendJson = JObject.FromObject(new
					{
						head = "Event",
						content = new
						{
							head = "Chat",
							content = new
							{
								senderId = sessionId,
								sender = playersData[sessionId].Name,
								message = $"{chatContent}",
							},
						}
					});

					// チャットを送信者を含め全員に送信して回る
					foreach (var sendId in _sessionManager.Sessions)
					{
						_ = SendTo(sendId, $"{sendJson}");
					}
				}

				void Join(int sessionId)
				{
					playersStatus.TryAdd(sessionId, new PlayerStatus
					{
						PositionX = 0,
						PositionY = 0,
						PositionZ = 0,
					});
					
					var joinedData = playersData.AddOrUpdate(sessionId, new PlayerData
					{
						Name = NameGenerator.Generate(sessionId),
						Color = 0x00ff00,
					},
					(oldId, data) => 
					{
						return data;
					});

					JObject sendJson = JObject.FromObject(new
					{
						head = "Event",
						content = new
						{
							head = "Chat",
							content = new
							{
								senderId = -1,
								sender = "*",
								message = $"{joinedData.Name}さんが参加しました。",
							}
						},
					});

					foreach (var sendId in _sessionManager.Sessions)
					{
						// MEMO: 参加したてのIdさんを含む全員に送信する
						_ = SendTo(sendId, $"{sendJson}");
					}
				}
			}

			try
			{
				Log.WriteLine($"GameLogic起動した");

				while (true)
				{
					foreach (var id in _sessionManager.Sessions)
					{
						if (_sessionManager.TryDequeue(id, out var data))
						{
							try
							{
								RequestProc(id, data);
							}
							catch (Exception ex)
							{
								Log.Error($"TCP受信処理で例外:{ex}");
							}
						}

						if (_sessionManager.TryDequeueUDP(id, out var udpData))
						{
							try
							{
								RequestProc(id, udpData);
							}
							catch (Exception ex)
							{
								Log.Error($"UDP受信処理で例外:{ex}");
							}
						}
					}
					//await Task.Delay(1 /* TODO: マジックナンバー消す */, cts.Token);
				}
			}
			catch (OperationCanceledException)
			{
				Log.WriteLine($"GameLogicキャンセルを受信した");
			}
			catch (Exception ex)
			{
				Log.Error($"{ex}");
			}

			Log.WriteLine($"GameLogic停止した");
		});
	}
}

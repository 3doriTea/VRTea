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

namespace VRTeaServer.Service
{
	internal class GameLogicService : IService
	{
		private readonly SessionManager _sessionManager;

		/// <summary>
		/// しょっちゅう変わるプレイヤー情報
		/// </summary>
		private class PlayerStatus
		{
			public float PositionX { get; set; } = 0.0f;
			public float PositionY { get; set; } = 0.0f;
			public float PositionZ { get; set; } = 0.0f;
		}

		/// <summary>
		/// あまり変わらないプレイヤー情報
		/// </summary>
		private class PlayerData
		{
			public uint Color { get; set; } = 0xffffffff;
			public string Name { get; set; } = string.Empty;
		}
		
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
		private readonly ConcurrentBag<TextChat> textChats = [];
		private uint _textChatIdCounter = 0;

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

			_sessionManager.OnDisconnected += (ipEndPoint, leavedId) =>
			{
				string leavedUserName = $"{leavedId}";

				JObject sendJson = JObject.FromObject(new
				{
					head = "Event",
					content = $"{leavedUserName}さんが退出しました。",
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
					case "Update":
						break;
					case "Event":
						//var eventContent = json.GetValue()
						break;
					case "Join":
						Join(sessionId);
						break;
					default:
						Log.WriteLine($"SID:{sessionId} から不明な head:{head}");
						break;
				}

				void EventChat(int sessionId)
				{
					
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
						content = $"{joinedData.Name}さんが参加しました。",
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
				while (true)
				{
					foreach (var id in _sessionManager.Sessions)
					{
						if (_sessionManager.TryDequeue(id, out var data))
						{
							RequestProc(id, data);
						}
					}
				}
			}
			catch (OperationCanceledException)
			{
				return;
			}
		});
	}
}

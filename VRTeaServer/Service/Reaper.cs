using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VRTeaServer.Logging;
using VRTeaServer.Utility;

namespace VRTeaServer.Service
{
	internal class Reaper : IService
	{
		/// <summary>
		/// 切断判定をする時間差 (秒)
		/// </summary>
		public float IntervalSec { get; set; } = 5.0f;

		private readonly SessionManager _sessionManager;

		public Reaper(SessionManager sessionManager)
		{
			_sessionManager = sessionManager;
		}

		public async Task Start(CancellationTokenSource cts)
		{
			try
			{
				Log.WriteLine($"Reaper起動した");

				while (true)
				{
					// 切断予定のセッションId
					var toRemoveIds = new List<int>();

					var now = DateTime.Now;
					await Task.Delay(TimeUtil.ToMilliSec(IntervalSec), cts.Token);
					foreach (var sessionId in _sessionManager.Sessions)
					{
						if (_sessionManager.TryGet(sessionId, cts, out var session))
						{
							if (session is null)
							{
								Log.WriteLine("取得に成功したはずの sessionが nullだった…");
								continue;
							}

							TimeSpan diff = now - session.Timestamp;
							if (diff.TotalSeconds > IntervalSec)
							{
								// 前回の更新から IntervalSec 以上経過しているため切断判定
								session.ToDestroyFlag = true;
							}

							if (session.ToDestroyFlag)
							{
								toRemoveIds.Add(sessionId);
							}
						}
						else
						{
							Log.WriteLine("Reaper: セッションIdからのセッションの取得に失敗");
						}
					}

					// 切断予定のセッションを切断してく
					foreach(var id in toRemoveIds)
					{
						_sessionManager.Disconnect(id);
					}
				}
			}
			catch (OperationCanceledException)
			{
				Log.WriteLine($"Reaperキャンセル受信した");
				return;  // キャンセル発動なら普通に終了
			}
			catch (Exception ex)
			{
				Log.Error($"Reaperで例外発生:\n{ex}");
			}

			Log.WriteLine($"Reaper停止した");
		}
	}
}

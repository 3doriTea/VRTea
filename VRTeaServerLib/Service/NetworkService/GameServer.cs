using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib.Service.NetworkService
{
	public class GameServer : INetworkService
	{
		public class Config
		{
			public int BufferSize{ get; init; }
			public float ReceiveIntervalSec{ get; init; }
		}

		public TcpListener Listener { get; }
		public SessionManager.Session.Mode SessionMode => SessionManager.Session.Mode.Game;
		private SessionManager _sessionManager;
		private Config _config;

		public GameServer(SessionManager sessionManager, IPAddress ipAddress, ushort port, Config config)
		{
			_sessionManager = sessionManager;
			Listener = new TcpListener(ipAddress, port);
			_config = config;
		}

		public async Task SessionClientAsync(int id, CancellationTokenSource cts)
		{
			var session = _sessionManager.GetSession(id);

			var stream = session.Client.GetStream();
			byte[] buffer = new byte[_config.BufferSize];

			try
			{
				await Task.WhenAll(
					Task.Run(async () =>
					{
						while (true)
						{
							// TODO: 頭サイズ、ボディにする
							int bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);
							if (bytesRead == 0)
							{
								session.ToDestroyFlag = true;
								break;
							}
							var receiveData = new SessionManager.ReceiveData(buffer.AsSpan(0, bytesRead).ToArray());
							session.ReceiveQueue.Enqueue(receiveData);
							session.Timestamp = DateTime.Now;
						}
					}, cts.Token),
					Task.Run(async () => 
					{
						while (await session.SendQueue.Reader.WaitToReadAsync(cts.Token))
						{
							while (session.SendQueue.Reader.TryRead(out var sendData))
							{
								await stream.WriteAsync(sendData.buffer, cts.Token);
							}

							// OSくん、さっさと送信しろ！(圧)
							await stream.FlushAsync();
							// MEMO: 小さいデータは一度溜め、次の機会に一気に送ることで効率を上げるOSや.NETの仕様があるそうです。
							//     : 効率重視でリアルタイム性に欠けるのはよろしくないので、強制的に流すよう命令します。
						}
					}));
			}
			catch (OperationCanceledException)
			{
				return;
			}
			catch (IOException ex)
			{
				
			}
		}
	}
}

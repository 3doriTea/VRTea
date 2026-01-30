using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using VRTeaServer.Logging;

namespace VRTeaServer.Service
{
	/// <summary>
	/// TCPを使うゲームサービス
	/// </summary>
	internal class GameTcpService : IService
	{
		private readonly SessionManager _sessionManager;
		private IPAddress _serverIPAddress;  // サーバーのIPアドレス
		private ushort _gamePort;            // ゲームサービスを公開するポート番号
		const int BufferSize = 1024;


		public GameTcpService(SessionManager sessionManager, IPAddress serverIPAddress, ushort gamePort)
		{
			_sessionManager = sessionManager;
			_serverIPAddress = serverIPAddress;
			_gamePort = gamePort;
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(async () =>
		{
			using var listener = new TcpListener(_serverIPAddress, _gamePort);

			listener.Start();

			Log.WriteLine($"サーバー起動した");
			while (true)
			{
				try
				{
					TcpClient client = await listener.AcceptTcpClientAsync(cts.Token);
					// クライアント受け付けたら即セッション開始
					_ = StartSession(client, cts);
				}
				catch (OperationCanceledException)
				{
					Log.WriteLine($"サーバーキャンセルを受信した");
					break;  // キャンセル発動されたらサービス止める
				}
			}
			Log.WriteLine($"サーバー停止した");
			listener.Stop();
			listener.Dispose();
		});

		private async Task StartSession(TcpClient client, CancellationTokenSource cts)
		{
			int sessionId = _sessionManager.BeginSession(client, cts, SessionMode.Game);
			Log.WriteLine($"クライアント受け付けた{client.Client.RemoteEndPoint}");

			try
			{
				NetworkStream stream = client.GetStream();
				byte[] buffer = new byte[BufferSize];

				try
				{
					await Task.WhenAll(
						Task.Run(async () =>
						{
							int bytesRead = 0;
							while ((bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length, cts.Token)) > 0)
							{
								_sessionManager.
								session.RecvQueue.Enqueue(Encoding.UTF8.GetString(buffer, 0, bytesRead));
							}
						}, cts.Token),
						Task.Run(async () =>
						{
							byte[]? sendBuffer = null;
							while (true)
							{
								while (session.SendQueue.TryDequeue(out sendBuffer))
								{
									await stream.WriteAsync(sendBuffer, cts.Token);
								}
								await Task.Delay(10, cts.Token);
							}
						}, cts.Token));

					OnDisconnected.Invoke(id);
				}
				catch (OperationCanceledException ex)
				{
					_ = ex;
					return;
				}
				catch (IOException ex)
				{
					Console.WriteLine($"{ex}");
				}
			}
		}
	}
}

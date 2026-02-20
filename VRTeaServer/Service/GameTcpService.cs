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
		const int BufferSize = 1024;         // 溜めておくバッファのサイズ

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

		/// <summary>
		/// クライアントとセッション開始
		/// </summary>
		/// <param name="client"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
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
								await _sessionManager.ReceiveEnqueue(sessionId, new ReceiveData(buffer.AsSpan(0..bytesRead).ToArray()), cts);
							}
						}, cts.Token),
						Task.Run(async () =>
						{
							while (true)
							{
								SendData sendData = await _sessionManager.SendDequeue(sessionId, cts);
								await stream.WriteAsync(sendData.Buffer, cts.Token);
							}
						}, cts.Token));

					// クライアントの受付終わったから切断する
					_sessionManager.Disconnect(sessionId);
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
			catch (Exception ex)
			{
				Log.Error($"{ex}");
			}
		}
	}
}

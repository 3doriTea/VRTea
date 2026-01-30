using System;
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
					_sessionManager.BeginSession(client, cts, SessionMode.Game);

					Log.WriteLine($"クライアント受け付けた{client.Client.RemoteEndPoint}");
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
	}
}

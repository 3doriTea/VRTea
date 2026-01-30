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
	/// TCPを使うWebサービス
	/// </summary>
	internal class WebTcpService : IService
	{
		private readonly SessionManager _sessionManager;
		private IPAddress _serverIPAddress;  // サーバーのIPアドレス
		private ushort _webPort;             // Webサービスを公開するポート番号
		const int BufferSize = 1024;


		public WebTcpService(SessionManager sessionManager, IPAddress serverIPAddress, ushort webPort = 80)
		{
			_sessionManager = sessionManager;
			_serverIPAddress = serverIPAddress;
			_webPort = webPort;
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(async () =>
		{
			using var listener = new TcpListener(_serverIPAddress, _webPort);

			listener.Start();

			Log.WriteLine($"Webサーバー起動した");
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
					Log.WriteLine($"Webサーバーキャンセルを受信した");
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

			NetworkStream stream = client.GetStream();
			byte[] buffer = new byte[BufferSize];

			try
			{
				int bytesRead = 0;
				while ((bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length, cts.Token)) > 0)
				{
					break;  // TODO: Webの担当の方実装してください。
				}
			}
			catch (OperationCanceledException)
			{
			}
			catch (IOException ex)
			{
				Console.WriteLine($"{client.Client}:{ex}");
			}
			finally
			{
				_sessionManager.EndSession(sessionId);
			}
		}
	}
}

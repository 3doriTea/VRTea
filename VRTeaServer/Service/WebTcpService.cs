using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Reflection.PortableExecutable;
using System.Text;
using System.Text.Encodings.Web;
using System.Text.Json;
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
			listener.Stop();
			listener.Dispose();
			Log.WriteLine($"Webサーバー停止した");
		});

		private async Task StartSession(TcpClient client, CancellationTokenSource cts)
		{
			int sessionId = _sessionManager.BeginSession(client, cts, SessionMode.Game);
			Log.WriteLine($"クライアント受け付けた{client.Client.RemoteEndPoint}");

			using NetworkStream stream = client.GetStream();
			using StreamReader reader	= new StreamReader(stream,Encoding.UTF8);
			try
			{
				string? line ="";
				
                StringBuilder requestBuilder = new();
                while (!string.IsNullOrEmpty(line = await reader.ReadLineAsync()))
                {
                    requestBuilder.AppendLine(line);
                }
                string request = requestBuilder.ToString();
                Log.WriteLine("クライアントからリクエスト受信\r\n" + request);

				int sessionCount = _sessionManager.Sessions.Count;
				string body = $"{{\"count\":{sessionCount}}}";
				string response = 
                    "HTTP/1.1 200 OK\r\n" +
                    "Content-Type: application/json; charset=UTF-8\r\n" +
                    "Access-Control-Allow-Origin: *\r\n" +
                    $"Content-Length: {Encoding.UTF8.GetByteCount(body)}\r\n" +
                    "Connection: close\r\n" +
                    "\r\n" + 
                    body;
                
				byte[] data = Encoding.UTF8.GetBytes(response);
				await stream.WriteAsync(data, 0, data.Length);

				Log.WriteLine("クライアントへレスポンス送信\r\n" + response);				
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

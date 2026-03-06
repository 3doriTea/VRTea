using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Web;
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

				// リクエストを一行ずつ読み取る
                while (!string.IsNullOrEmpty(line = await reader.ReadLineAsync()))
                {
                    requestBuilder.AppendLine(line);
                }
                string request = requestBuilder.ToString();
                Log.WriteLine("クライアントからリクエスト受信\r\n" + request);
				// クエリを解析
				var queries = ParseRequest(request);
				if(queries == null)
				{
					return;
				}
				
				if( queries["query"] == "clientCount")
				{
                    Log.WriteLine("クエリを取得\r\n" + queries["query"]);

					// クライアント数
                    int sessionCount = _sessionManager.Sessions.Count;

					// レスポンス作成
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

        /// <summary>
        /// HTTPリクエストを解析してクエリパラメータを返す
        /// </summary>
        /// <param name="request"></param>
        /// <returns>キー(パラメータの名前)と値がペアのコレクションを返す</returns>
        static private System.Collections.Specialized.NameValueCollection? ParseRequest(string request)
		{
			// HTTPリクエストをヘッダーとボディに分ける
            var parts = request.Split(new[] { "\r\n\r\n" }, 2, StringSplitOptions.None);
			// ヘッダー部分
            string headerPart = parts[0];

			// ヘッダーを一行ごとに分ける
            var headerLines = headerPart.Split("\r\n");

			// リクエスト行をスペースで区切って取得
            List<string> requestLine = headerLines[0].Split(" ").ToList();

			// クエリ文字列を取得
			string queryParams = requestLine[1];
			if (queryParams.Contains('?'))
			{
				return HttpUtility.ParseQueryString(queryParams.Substring(queryParams.IndexOf('?')));
			}
			return null;
        }
	}
}
